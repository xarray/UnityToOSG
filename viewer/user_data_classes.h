#ifndef H_UNITY2OSG_USERDATACLASSES
#define H_UNITY2OSG_USERDATACLASSES

#include <osg/io_utils>
#include <osg/Program>
#include <osg/StateSet>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

struct LightUniforms
{
    osg::ref_ptr<osg::Uniform> color;
    osg::ref_ptr<osg::Uniform> direction;
};

struct ShaderDataProxy : public osg::Program
{
    virtual osg::Object* cloneType() const { return new ShaderDataProxy; }
    virtual const char* libraryName() const { return "nwTools"; }
    virtual const char* className() const { return "ShaderData"; }
    
    std::string shaderName;
};

struct TerrainDataProxy : public osg::Geometry
{
    virtual osg::Object* cloneType() const { return new TerrainDataProxy; }
    virtual const char* libraryName() const { return "nwTools"; }
    virtual const char* className() const { return "Terrain"; }
    
    std::vector<float> heightMap;
    std::vector<osg::Vec4ub> alphaMaps;
    std::vector<std::string> splatTextures;
    std::vector<osg::Vec4> splatTilingOffsets;
    osg::Vec2s heightMapSize, alphaMapSize;
    osg::Vec3 size;
    int layers;
};

typedef std::map<ShaderDataProxy*, osg::StateSet*> ShaderDataProxyMap;
typedef std::map<TerrainDataProxy*, osg::Geode*> TerrainDataProxyMap;

extern void applyUserShaders( ShaderDataProxyMap& sd, const std::string& dbPath );
extern void applyUserTerrains( TerrainDataProxyMap& td, const std::string& dbPath );

#endif
