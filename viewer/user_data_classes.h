#ifndef H_UNITY2OSG_USERDATACLASSES
#define H_UNITY2OSG_USERDATACLASSES

#include <osg/io_utils>
#include <osg/Program>
#include <osg/StateSet>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include "Particle.h"

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

struct ParticleDataProxy : public osg::Geode
{
    virtual osg::Object* cloneType() const { return new ParticleDataProxy; }
    virtual const char* libraryName() const { return "nwTools"; }
    virtual const char* className() const { return "ParticleSystem"; }
    
    // Basic attributes
    float duration, playingSpeed;
    int maxParticles;
    bool isLooping, isAutoStarted;
    osg::Vec3 gravity, rotation;
    osg::Vec4 startAttributes;  // [life, size, speed, delay]
    osg::Vec4 startColor;
    
    // Emission module  // TODO: bursts, curve data
    std::vector<osg::Vec4> emissionRate;  // [time, value, inTangent, outTangent]
    std::string emissionType;
    
    // Texture Sheet Animation module  // TODO: curve data
    int tsaCycleCount;
    osg::Vec2 tsaNumTiles;
    std::vector<osg::Vec4> tsaFrameOverTime;  // [time, value, inTangent, outTangent]
    std::string tsaAnimationType;
    
    // Renderer module  // TODO: render as mesh
    std::vector<std::string> renderTextures;
    std::vector<osg::Vec4> renderTilingOffsets;
    osg::Vec4 renderAttributes;  // [minSize, maxSize, normalDir, sortFudge]
    std::string renderShapeMode, renderSortMode;
};

typedef std::map<ShaderDataProxy*, osg::StateSet*> ShaderDataProxyMap;
typedef std::map<TerrainDataProxy*, osg::Geode*> TerrainDataProxyMap;
typedef std::map<ParticleDataProxy*, osg::Group*> ParticleDataProxyMap;

extern void applyUserShaders( ShaderDataProxyMap& sd, const std::string& dbPath );
extern void applyUserTerrains( TerrainDataProxyMap& td, const std::string& dbPath );
extern void applyUserParticles( ParticleDataProxyMap& pd, nwCore::SparkUpdatingHandler*, const std::string& dbPath );

#endif
