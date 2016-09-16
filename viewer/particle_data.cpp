#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osgParticle/Particle>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/RadialShooter>

#include <algorithm>
#include "user_data_classes.h"

void applyUserParticles( ParticleDataProxyMap& sdMap, const std::string& dbPath )
{
    static std::map< std::string, osg::ref_ptr<osg::Program> > s_sharedParticlePrograms;
    for ( ParticleDataProxyMap::iterator itr=sdMap.begin(); itr!=sdMap.end(); ++itr )
    {
        //
    }
}

bool ParticleData_readLocalData( osg::Object& obj, osgDB::Input& fr )
{
    ParticleDataProxy& proxy = static_cast<ParticleDataProxy&>(obj);
    bool iteratorAdvanced = false;
    
    if ( fr.matchSequence("Duration %f") )
    {
        fr[1].getFloat( proxy.duration );
        iteratorAdvanced = true; fr += 2;
    }
    
    // TODO
    return iteratorAdvanced;
}

bool ParticleData_writeLocalData( const osg::Object&, osgDB::Output& )
{ return false; }

REGISTER_DOTOSGWRAPPER( ParticleData_Proxy )
(
    new ParticleDataProxy,
    "nwTools::ParticleSystem",
    "Object Node Geode nwTools::ParticleSystem",
    ParticleData_readLocalData, ParticleData_writeLocalData
);
