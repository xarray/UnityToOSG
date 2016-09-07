#ifndef H_UNITY2OSG_USERDATACLASSES
#define H_UNITY2OSG_USERDATACLASSES

#include <osg/Program>
#include <osg/StateSet>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

struct ShaderDataProxy : public osg::Program
{
    virtual osg::Object* cloneType() const { return new ShaderDataProxy; }
    virtual const char* libraryName() const { return "nwTools"; }
    virtual const char* className() const { return "ShaderData"; }
    
    std::string shaderName;
};

typedef std::map<ShaderDataProxy*, osg::StateSet*> ShaderDataProxyMap;

extern void applyUserShaders( ShaderDataProxyMap& sd, const std::string& dbPath );

#endif
