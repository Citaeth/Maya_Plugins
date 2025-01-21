#include <maya/MPxNode.h>
#include <maya/MTypeId.h>


class CacheSequencer : public MPxNode {
public:
    CacheSequencer();
    ~CacheSequencer() override;
    MStatus compute(const MPlug& plug, MDataBlock& block) override;
    static void* creator();
    static MStatus initialize();
    static MTypeId id;
private:
    static MObject usdCache;
    static MObject path;
    static MObject referenceEdit;
    static MObject startFrame;
    static MObject endFrame;
    static MObject usdCacheStart;
    static MObject output;
    static MObject oPath;
    static MObject offset;
    static MObject time;
};