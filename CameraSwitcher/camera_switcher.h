#include <maya/MPxNode.h>
#include <maya/MTypeId.h>


class CameraSwitcher : public MPxNode {
public:
    CameraSwitcher();
    ~CameraSwitcher() override;
    MStatus compute(const MPlug& plug, MDataBlock& block) override;
    static void* creator();
    static MStatus initialize();
    static MTypeId id;
private:
    static MObject Camera;
    static MObject name;
    static MObject startFrame;
    static MObject endFrame;
    static MObject output;
    static MObject time;
};