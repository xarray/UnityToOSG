#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <algorithm>
#include "user_data_classes.h"

void applyUserShaders( ShaderDataProxyMap& sdMap, const std::string& dbPath )
{
    static std::map< std::string, osg::ref_ptr<osg::Program> > s_sharedShaderPrograms;
    for ( ShaderDataProxyMap::iterator itr=sdMap.begin(); itr!=sdMap.end(); ++itr )
    {
        std::string name = itr->first->shaderName;
        if ( s_sharedShaderPrograms.find(name)==s_sharedShaderPrograms.end() )
        {
            std::string vertFile, fragFile, shaderPath = dbPath + "/shaders/" + name;
            std::replace( shaderPath.begin(), shaderPath.end(), ' ', '_');
            vertFile = osgDB::findDataFile(shaderPath + ".vert");
            fragFile = osgDB::findDataFile(shaderPath + ".frag");
            if ( vertFile.empty() ) vertFile = dbPath + "/shaders/Default.vert";
            if ( fragFile.empty() )
            {
                fragFile = dbPath + "/shaders/Default.frag";
                OSG_NOTICE << "Missed fragment shader for " << name << std::endl;
            }
            
            osg::ref_ptr<osg::Program> program = new osg::Program;
            program->addShader( osgDB::readShaderFile(osg::Shader::VERTEX, vertFile) );
            program->addShader( osgDB::readShaderFile(osg::Shader::FRAGMENT, fragFile) );
            program->addBindAttribLocation( "tangent", 6 );
            s_sharedShaderPrograms[name] = program;
        }
        itr->second->removeAttribute( itr->first );
        itr->second->setAttributeAndModes( s_sharedShaderPrograms[name].get() );
    }
}

bool ShaderData_readLocalData( osg::Object& obj, osgDB::Input& fr )
{
    ShaderDataProxy& proxy = static_cast<ShaderDataProxy&>(obj);
    bool iteratorAdvanced = false;
    
    if ( fr.matchSequence("ShaderName %s") )
    {
        proxy.shaderName = fr[1].getStr();
        iteratorAdvanced = true; fr += 2;
    }
    
    // TODO
    return iteratorAdvanced;
}

bool ShaderData_writeLocalData( const osg::Object&, osgDB::Output& )
{ return false; }

REGISTER_DOTOSGWRAPPER( ShaderData_Proxy )
(
    new ShaderDataProxy,
    "nwTools::ShaderData",
    "Object StateAttribute Program nwTools::ShaderData",
    ShaderData_readLocalData, ShaderData_writeLocalData
);
