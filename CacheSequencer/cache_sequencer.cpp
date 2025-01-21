
#include <maya/MDataHandle.h>
#include <maya/MPlug.h>
#include <maya/MFnPlugin.h>
#include <maya/MTime.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnStringData.h>
#include <maya/MGlobal.h>
#include "cache_sequencer.h"
#include <iostream>


MTypeId CacheSequencer::id(0x00128076);
MObject CacheSequencer::usdCache;
MObject CacheSequencer::path;
MObject CacheSequencer::startFrame;
MObject CacheSequencer::endFrame;
MObject CacheSequencer::usdCacheStart;
MObject CacheSequencer::time;
MObject CacheSequencer::output;
MObject CacheSequencer::offset;
MObject CacheSequencer::oPath;

CacheSequencer::CacheSequencer() {}
CacheSequencer::~CacheSequencer() {}
void* CacheSequencer::creator() { return new CacheSequencer(); }


MStatus CacheSequencer::compute(const MPlug& plug, MDataBlock& dataBlock) {
    if (plug != this->output && plug != this->oPath && plug != this->offset) {
        return MS::kUnknownParameter;
    }
    MDataHandle currentTimeHandle = dataBlock.inputValue(this->time);
    MDataHandle offsetHandle = dataBlock.outputValue(this->output);
    MDataHandle oPathHandle = dataBlock.outputValue(this->oPath);
    MTime currentTime = currentTimeHandle.asTime();
    currentTime.setUnit(MTime::uiUnit());

    MArrayDataHandle UsdArrayHandle = dataBlock.inputArrayValue(this->usdCache);
    for (unsigned int i(0); i < UsdArrayHandle.elementCount(); ++i) {
        UsdArrayHandle.jumpToArrayElement(i);
        MDataHandle UsdHandle = UsdArrayHandle.inputValue();

        MTime startTime(UsdHandle.child(this->startFrame).asTime());
        startTime.setUnit(MTime::uiUnit());
        MTime endTime(UsdHandle.child(this->endFrame).asTime());
        endTime.setUnit(MTime::uiUnit());

        if (startTime.value() == endTime.value()) {
            continue;
        }

        if (currentTime.value() >= startTime.value() && currentTime.value() <= endTime.value()) {
            MDataHandle UsdStartHandle = UsdHandle.child(this->usdCacheStart);
            MDataHandle pathHandle = UsdHandle.child(this->path);
            float offsetValue = -(startTime.value() - UsdStartHandle.asDouble());
            offsetHandle.setDouble(offsetValue);
            offsetHandle.setClean();
            oPathHandle.setString(pathHandle.asString());
            oPathHandle.setClean();
            return MS::kSuccess;
        }
    }
    offsetHandle.setDouble(0);
    offsetHandle.setClean();
    oPathHandle.setString("");
    oPathHandle.setClean();
    return MS::kSuccess;
}


MStatus CacheSequencer::initialize() {
    MStatus status;
    MStatus status2;
    MFnNumericAttribute fnNumAttr;
    MFnCompoundAttribute fnCompoundAttr;
    MFnStringData fnStringData;
    MFnUnitAttribute fnUnitAttr;
    MFnTypedAttribute fnTypedAttr;

    usdCacheStart = fnNumAttr.create("usdCacheStart", "ucs", MFnNumericData::kDouble);
    fnNumAttr.setConnectable(true);
    fnNumAttr.setReadable(false);
    fnNumAttr.setStorable(true);

    path = fnTypedAttr.create("path", "p", MFnData::kString, fnStringData.create(&status2), &status);
    fnTypedAttr.setConnectable(false);
    fnTypedAttr.setReadable(false);
    fnTypedAttr.setKeyable(false);
    fnTypedAttr.setStorable(true);

    startFrame = fnUnitAttr.create("startFrame", "sf", MFnUnitAttribute::kTime);
    fnUnitAttr.setConnectable(true);
    fnUnitAttr.setWritable(true);
    fnUnitAttr.setStorable(false);
    fnUnitAttr.setReadable(false);

    endFrame = fnUnitAttr.create("endFrame", "ef", MFnUnitAttribute::kTime);
    fnUnitAttr.setConnectable(true);
    fnUnitAttr.setWritable(true);
    fnUnitAttr.setStorable(false);
    fnUnitAttr.setReadable(false);

    usdCacheStart = fnNumAttr.create("usdCacheStartFrame", "mcs", MFnNumericData::kDouble);
    fnNumAttr.setConnectable(true);
    fnNumAttr.setWritable(true);
    fnNumAttr.setStorable(true);
    fnNumAttr.setReadable(false);

    usdCache = fnCompoundAttr.create("usdCache", "usdCache");
    fnCompoundAttr.addChild(path);
    fnCompoundAttr.addChild(usdCacheStart);
    fnCompoundAttr.addChild(startFrame);
    fnCompoundAttr.addChild(endFrame);
    fnCompoundAttr.setArray(true);
    addAttribute(usdCache);

    offset = fnNumAttr.create("offset", "of", MFnNumericData::kDouble);
    fnNumAttr.setConnectable(true);
    fnNumAttr.setReadable(true);
    fnNumAttr.setStorable(false);
    fnNumAttr.setWritable(false);

    oPath = fnTypedAttr.create("outputPath", "op", MFnData::kString, fnStringData.create(&status2), &status);
    fnTypedAttr.setConnectable(true);
    fnTypedAttr.setReadable(true);
    fnTypedAttr.setStorable(false);
    fnTypedAttr.setKeyable(false);
    fnTypedAttr.setWritable(false);

    output = fnCompoundAttr.create("output", "os");
    fnCompoundAttr.addChild(offset);
    fnCompoundAttr.addChild(oPath);
    fnCompoundAttr.setWritable(false);
    addAttribute(output);

    time = fnUnitAttr.create("time", "t", MFnUnitAttribute::kTime);
    fnUnitAttr.setWritable(true);
    fnUnitAttr.setReadable(false);
    addAttribute(time);

    attributeAffects(time, oPath);
    attributeAffects(time, offset);
    attributeAffects(path, oPath);
    attributeAffects(usdCacheStart, offset);
    attributeAffects(startFrame, offset);
    attributeAffects(endFrame, offset);
    attributeAffects(startFrame, oPath);
    attributeAffects(endFrame, oPath);
    attributeAffects(usdCacheStart, offset);
    return MS::kSuccess;
}


MStatus initializePlugin(MObject object) {
    MStatus status;
    MFnPlugin fnPlugin(object, "CacheSequencer", "0.0", "Any");
    status = fnPlugin.registerNode(
        "CacheSequencer",
        CacheSequencer::id,
        CacheSequencer::creator,
        CacheSequencer::initialize);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MGlobal::displayInfo("Usd Sequencer loaded");
    return status;
}


MStatus uninitializePlugin(MObject object) {
    MStatus status;
    MFnPlugin fnPlugin(object);
    status = fnPlugin.deregisterNode(CacheSequencer::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MGlobal::displayInfo("Usd Sequencer unloaded");
    return status;
}