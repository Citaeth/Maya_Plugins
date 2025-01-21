#include <maya/MDataHandle.h>
#include <maya/MPlug.h>
#include <maya/MFnPlugin.h>
#include <maya/MTime.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnStringData.h>
#include <maya/MGlobal.h>
#include "camera_switcher.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>


MTypeId CameraSwitcher::id(0x00128077);
MObject CameraSwitcher::Camera;
MObject CameraSwitcher::name;
MObject CameraSwitcher::startFrame;
MObject CameraSwitcher::endFrame;
MObject CameraSwitcher::time;
MObject CameraSwitcher::output;

// log messages
std::stringstream ss;

/*
    LineStep calculates the linear step from 0 to 1 from the start value to the end value.
    It will clamp the value to 0 if the range is less of the mean value and to 1 if the range is out of the max value.

*/
double LineStep(double value, double start, double end)
{
    if (!(start < end))
    {
        return 0.0;
    }
    double step = (value - start) / (end - start);

    if (step <= 0.0)
    {
        return 0.0;
    }
    if (step >= 1.0)
    {
        return 1.0;
    }
    return step;
}

/*
    this structure helps to arrange the ranges into objects so it can be queried easily, it adds some properties
    and functions to work with other frame range objects.
    the properties define the state of the frame range, it will take account if the frame range is intersecting other,
    it theres an gap before or after the current range.
*/
struct FrameRangeObject
{
    FrameRangeObject(double start_frame,
        double end_frame,
        int index
    ) : start(start_frame), end(end_frame), dataIndex(index) {
    }

    double start;
    double end;

    int dataIndex;

    bool headIntersection = false;
    double headStart;
    double headEnd;

    bool tailIntersection = false;
    double tailStart;
    double tailEnd;

    bool headGap = false;
    double headGapStart;
    double headGapEnd;

    bool tailGap = false;
    double tailGapStart;
    double tailGapEnd;

    /*
        Update the frame range accordingly to the location in the frame range list, adding the range for
        transitions is the range is intersecting or extending the frame range in case it has gaps.
    */
    void UpdateRangeStartEnd()
    {
        MGlobal::displayInfo(ss.str().c_str());
        std::vector<double> minValues;
        std::vector<double> maxValues;

        double minValue;
        double maxValue;

        minValues.push_back(start);
        if (headIntersection)
            minValues.push_back(headStart);
        if (headGap)
            minValues.push_back(headGapStart);

        maxValues.push_back(end);
        if (tailIntersection)
            maxValues.push_back(tailEnd);
        if (tailGap)
            maxValues.push_back(tailGapEnd);

        if (!minValues.empty())
            start = *std::min_element(minValues.begin(), minValues.end());

        if (!maxValues.empty())
            end = *std::max_element(maxValues.begin(), maxValues.end());

    }

    /*
     Compares the current frame range to another one, and updates both objects accordingly to the relation they could
     have, the current cases are intersection, and gaps
    */
    void calculateRelationship(FrameRangeObject& rangeObject)
    {
        double intersection = rangeObject.start - end;
        if (intersection <= double(0.0))
        {
            // updating this data
            tailIntersection = true;
            tailStart = rangeObject.start;
            tailEnd = end;

            // updating range object data
            rangeObject.headIntersection = true;
            rangeObject.headStart = rangeObject.start;
            rangeObject.headEnd = end;
        }
        else
        {
            rangeObject.headGap = true;
            rangeObject.headGapStart = end;
            rangeObject.headGapEnd = rangeObject.start;

            tailGap = true;
            tailGapStart = end;
            tailGapEnd = rangeObject.start;
        }
        UpdateRangeStartEnd();
        rangeObject.UpdateRangeStartEnd();
    }
};


CameraSwitcher::CameraSwitcher() {}
CameraSwitcher::~CameraSwitcher() {}
void* CameraSwitcher::creator() { return new CameraSwitcher(); }

MStatus CameraSwitcher::compute(const MPlug& plug, MDataBlock& dataBlock) {
    if (plug != this->output && plug != this->output) {
        return MS::kUnknownParameter;
    }
    MDataHandle currentTimeHandle = dataBlock.inputValue(this->time);

    MTime currentTime = currentTimeHandle.asTime();
    currentTime.setUnit(MTime::uiUnit());
    MArrayDataHandle CameraArrayHandle = dataBlock.inputArrayValue(this->Camera);
    MArrayDataHandle outputDataHandle = dataBlock.outputArrayValue(output);
    MArrayDataBuilder outData(&dataBlock, output, CameraArrayHandle.elementCount());
    std::vector<FrameRangeObject> rangeObjects;
    if (CameraArrayHandle.elementCount() == 0)
    {
        return MS::kSuccess;
    }
    for (unsigned int i(0); i < CameraArrayHandle.elementCount(); ++i) {
        CameraArrayHandle.jumpToArrayElement(i);
        MDataHandle CameraHandle = CameraArrayHandle.inputValue();
        MTime startTime(CameraHandle.child(this->startFrame).asTime());
        startTime.setUnit(MTime::uiUnit());
        MTime endTime(CameraHandle.child(this->endFrame).asTime());
        endTime.setUnit(MTime::uiUnit());
        rangeObjects.push_back(FrameRangeObject(startTime.value(), endTime.value(), i));
    }

    // sort rangeObjects, this helps to manage intersections and gaps, as the comparison between ranges will be 
    // calculated from the first one to the next.    
    std::sort(rangeObjects.begin(), rangeObjects.end(), [](const FrameRangeObject& a, const FrameRangeObject& b) {return a.start < b.start;});

    // calculate relationship
    for (int i = 0; i < rangeObjects.size() - 1; i++)
    {
        rangeObjects[i].calculateRelationship(rangeObjects[i + 1]);
    }

    for (FrameRangeObject rangeObject : rangeObjects)
    {
        if (currentTime.value() >= rangeObject.start && currentTime.value() <= rangeObject.end)
        {
            double inRange = 1.0;
            double headGap, headStep, tailStep, tailGap;
            // calculating gap influence
            if (!rangeObject.headGap)
                headGap = 1.0;
            else
                headGap = LineStep(currentTime.value(), rangeObject.headGapStart, rangeObject.headGapEnd);
            // calculating head intersection intersection influence
            if (!rangeObject.headIntersection)
                headStep = 1.0;
            else
                headStep = LineStep(currentTime.value(), rangeObject.headStart, rangeObject.headEnd);
            // calculating  tail intersection influence
            if (!rangeObject.tailIntersection)
                tailStep = 1.0;
            else
                tailStep = 1.0 - LineStep(currentTime.value(), rangeObject.tailStart, rangeObject.tailEnd);
            // calculating tail gap
            if (!rangeObject.tailGap)
                tailGap = 1.0;
            else
                tailGap = 1.0 - LineStep(currentTime.value(), rangeObject.tailGapStart, rangeObject.tailGapEnd);

            MDataHandle outDataHandle = outData.addElement(rangeObject.dataIndex);
            outDataHandle.setDouble(inRange * headGap * tailGap * headStep * tailStep);
        }
        else
        {
            MDataHandle outDataHandle = outData.addElement(rangeObject.dataIndex);
            outDataHandle.setDouble(0.0);
        }
    }

    outputDataHandle.set(outData);
    outputDataHandle.setAllClean();
    return MS::kSuccess;
}


MStatus CameraSwitcher::initialize() {
    MStatus status;
    MStatus status2;
    MFnNumericAttribute fnNumAttr;
    MFnCompoundAttribute fnCompoundAttr;
    MFnStringData fnStringData;
    MFnUnitAttribute fnUnitAttr;
    MFnTypedAttribute fnTypedAttr;

    name = fnTypedAttr.create("cameraName", "n", MFnData::kString, fnStringData.create(&status2), &status);
    fnTypedAttr.setConnectable(false);
    fnTypedAttr.setWritable(true);
    fnTypedAttr.setKeyable(false);
    fnTypedAttr.setStorable(true);
    fnTypedAttr.setReadable(true);

    startFrame = fnUnitAttr.create("startFrame", "sf", MFnUnitAttribute::kTime);
    fnUnitAttr.setConnectable(true);
    fnUnitAttr.setWritable(true);
    fnUnitAttr.setKeyable(true);
    fnUnitAttr.setStorable(true);
    fnUnitAttr.setReadable(true);

    endFrame = fnUnitAttr.create("endFrame", "ef", MFnUnitAttribute::kTime);
    fnUnitAttr.setConnectable(true);
    fnUnitAttr.setWritable(true);
    fnUnitAttr.setKeyable(true);
    fnUnitAttr.setStorable(true);
    fnUnitAttr.setReadable(true);

    Camera = fnCompoundAttr.create("Camera", "Camera");
    fnCompoundAttr.addChild(name);
    fnCompoundAttr.addChild(startFrame);
    fnCompoundAttr.addChild(endFrame);
    fnCompoundAttr.setArray(true);
    fnCompoundAttr.setStorable(true);
    fnCompoundAttr.setWritable(true);
    fnCompoundAttr.setReadable(true);
    addAttribute(Camera);

    output = fnNumAttr.create("output", "os", MFnNumericData::kDouble);
    fnNumAttr.setConnectable(true);
    fnNumAttr.setWritable(true);
    fnNumAttr.setStorable(true);
    fnNumAttr.setReadable(true);
    fnNumAttr.setUsesArrayDataBuilder(true);
    fnNumAttr.setArray(true);
    addAttribute(output);

    time = fnUnitAttr.create("time", "t", MFnUnitAttribute::kTime);
    fnUnitAttr.setWritable(true);
    fnUnitAttr.setReadable(false);
    addAttribute(time);

    attributeAffects(time, output);
    attributeAffects(startFrame, output);
    attributeAffects(endFrame, output);
    return MS::kSuccess;
}


MStatus initializePlugin(MObject object) {
    MStatus status;
    MFnPlugin fnPlugin(object, "CameraSwitcher", "0.0", "Any");
    status = fnPlugin.registerNode(
        "CameraSwitcher",
        CameraSwitcher::id,
        CameraSwitcher::creator,
        CameraSwitcher::initialize);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MGlobal::displayInfo(" Camera Switcher loaded");
    return status;
}


MStatus uninitializePlugin(MObject object) {
    MStatus status;
    MFnPlugin fnPlugin(object);
    status = fnPlugin.deregisterNode(CameraSwitcher::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MGlobal::displayInfo(" Camera Switcher unloaded");
    return status;
}