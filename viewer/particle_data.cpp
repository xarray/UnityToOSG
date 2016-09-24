#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <algorithm>
#include "user_data_classes.h"

SPK::SPK_ID createUserSparkSystem( const nwCore::SparkDrawable* spark,
                                   const nwCore::SparkDrawable::TextureIDMap& textureIDMap,
                                   int screenWidth, int screenHeight )
{
    SPK::System* system = SPK::System::create();
    const ParticleDataProxy* proxy = dynamic_cast<const ParticleDataProxy*>( spark->getUserData() );
    if ( proxy )
    {
        GLuint sparkTex = 0;  // TODO: only the first texture?
        nwCore::SparkDrawable::TextureIDMap::const_iterator itr = textureIDMap.find("texture0");
        if ( itr!=textureIDMap.end() ) sparkTex = itr->second;
        
        SPK::GL::GLQuadRenderer* sparkRenderer = NULL;
        {
            sparkRenderer = SPK::GL::GLQuadRenderer::create();  // TODO: shape module
            sparkRenderer->setScale( proxy->startAttributes[1], proxy->startAttributes[1] );
            sparkRenderer->setTexturingMode( SPK::TEXTURE_2D );
            sparkRenderer->setTexture( sparkTex );
            sparkRenderer->setTextureBlending( GL_MODULATE );
            sparkRenderer->setBlending( SPK::BLENDING_ADD );
            sparkRenderer->enableRenderingHint( SPK::DEPTH_WRITE, false );
            sparkRenderer->setAtlasDimensions( proxy->tsaNumTiles[0], proxy->tsaNumTiles[1] );
        }
        
        SPK::Model* sparkModel = NULL;
        {
            int enabledFlag = SPK::FLAG_TEXTURE_INDEX;
            int mutableFlag = SPK::FLAG_TEXTURE_INDEX;
            int randomFlag = SPK::FLAG_NONE;
            int interpolatedFlag = SPK::FLAG_NONE;
            
            sparkModel = SPK::Model::create(enabledFlag, mutableFlag, randomFlag, interpolatedFlag);
            sparkModel->setParam( SPK::PARAM_TEXTURE_INDEX, 0.0f, proxy->tsaNumTiles[0] * proxy->tsaNumTiles[1] );
            sparkModel->setLifeTime( 1.0f, 1.0f );//proxy->startAttributes[0], proxy->startAttributes[0] );
            //sparkModel->setImmortal( true );
        }
        
        SPK::Point* sparkSource = NULL;
        {
            // TODO: birth pos
            sparkSource = SPK::Point::create();
        }
        
        SPK::RandomEmitter* sparkEmitter = NULL;
        {
            sparkEmitter = SPK::RandomEmitter::create();
            sparkEmitter->setZone( sparkSource );
            sparkEmitter->setForce( 0.0f, 0.0f );
            sparkEmitter->setTank( -1 );
            sparkEmitter->setFlow( 10.0f );
        }
        
        SPK::Group* sparkGroup = NULL;
        {
            sparkGroup = SPK::Group::create(sparkModel, proxy->maxParticles);
            sparkGroup->addEmitter( sparkEmitter );
            sparkGroup->setRenderer( sparkRenderer );
            sparkGroup->setGravity( SPK::Vector3D(proxy->gravity[0], proxy->gravity[1], proxy->gravity[2]) );
        }
        
        system->addGroup( sparkGroup );
        system->enableAABBComputing( true );
        const_cast<nwCore::SparkDrawable*>(spark)->setUserData( NULL );
    }
    return system->getSPKID();
}

void applyUserParticles( ParticleDataProxyMap& pdMap, nwCore::SparkUpdatingHandler* handler,
                         const std::string& dbPath )
{
    SPK::randomSeed = static_cast<unsigned int>( time(NULL) );
    SPK::System::setClampStep( true, 0.1f );
    SPK::System::useAdaptiveStep( 0.001f, 0.01f );
    
    for ( ParticleDataProxyMap::iterator itr=pdMap.begin(); itr!=pdMap.end(); ++itr )
    {
        osg::ref_ptr<nwCore::SparkDrawable> spark = new nwCore::SparkDrawable;
        spark->setUserData( itr->first );  // for reading in the creator function
        spark->setBaseSystemCreator( &createUserSparkSystem );
        spark->addParticleSystem();
        
        ParticleDataProxy* proxy = itr->first;
        for ( unsigned int i=0; i<proxy->renderTextures.size(); ++i )
        {
            osg::Image* image = osgDB::readImageFile(proxy->renderTextures[i]);
            if ( image ) image->flipVertical();
            
            std::stringstream ss; ss << "texture" << i;  // TODO: renderTilingOffsets? always RGBA?
            spark->addImage( ss.str(), image, GL_RGBA );
        }
        handler->addSpark( spark.get() );
        
        osg::ref_ptr<osg::Geode> particleGeode = new osg::Geode;
        particleGeode->addDrawable( spark.get() );
        particleGeode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        particleGeode->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
        particleGeode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        
        itr->second->removeChild( itr->first );
        itr->second->addChild( particleGeode.get() );
    }
}

static void handleMinMaxCurve( std::vector<osg::Vec4>& curveData, osgDB::Input& fr )
{
    int length = 0; fr[1].getInt( length );
    fr += 3;  // CurveName Len {
    for ( int l=0; l<length; ++l )
    {
        osg::Vec4 value;
        for ( int i=0; i<4; ++i ) fr[i].getFloat( value[i] );
        curveData.push_back( value ); fr += 4;
    }
    ++fr;  // }
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
    
    if ( fr.matchSequence("Playing %f %i %i") )
    {
        int value = 0;
        fr[1].getFloat( proxy.playingSpeed );
        fr[2].getInt(value); proxy.isLooping = (value > 0);
        fr[3].getInt(value); proxy.isAutoStarted = (value > 0);
        iteratorAdvanced = true; fr += 4;
    }
    
    if ( fr.matchSequence("MaxParticles %i") )
    {
        fr[1].getInt( proxy.maxParticles );
        iteratorAdvanced = true; fr += 2;
    }
    
    if ( fr.matchSequence("Gravity %f %f %f") )
    {
        for (int i=0; i<3; ++i) fr[1+i].getFloat( proxy.gravity[i] );
        iteratorAdvanced = true; fr += 4;
    }
    
    if ( fr.matchSequence("Rotation %f %f %f") )
    {
        for (int i=0; i<3; ++i) fr[1+i].getFloat( proxy.rotation[i] );
        iteratorAdvanced = true; fr += 4;
    }
    
    if ( fr.matchSequence("StartAttributes %f %f %f %f") )
    {
        for (int i=0; i<4; ++i) fr[1+i].getFloat( proxy.startAttributes[i] );
        iteratorAdvanced = true; fr += 5;
    }
    
    if ( fr.matchSequence("StartColor %f %f %f %f") )
    {
        for (int i=0; i<4; ++i) fr[1+i].getFloat( proxy.startColor[i] );
        iteratorAdvanced = true; fr += 5;
    }
    
    if ( fr.matchSequence("Emission {") )
    {
        fr += 2;
        if ( fr.matchSequence("Type %w") )
        {
            proxy.emissionType = fr[1].getStr();
            fr += 2;
        }
        
        if ( fr.matchSequence("Rate %i {") ) handleMinMaxCurve(proxy.emissionRate, fr);
        iteratorAdvanced = true; ++fr;
    }
    
    if ( fr.matchSequence("TextureSheetAnimation {") )
    {
        fr += 2;
        if ( fr.matchSequence("Type %w") )
        {
            proxy.tsaAnimationType = fr[1].getStr();
            fr += 2;
        }
        
        if ( fr.matchSequence("Tiles %f %f") )
        {
            fr[1].getFloat( proxy.tsaNumTiles[0] );
            fr[2].getFloat( proxy.tsaNumTiles[1] );
            fr += 3;
        }
        
        if ( fr.matchSequence("CycleCount %i") )
        {
            fr[1].getInt( proxy.tsaCycleCount );
            fr += 2;
        }
        
        if ( fr.matchSequence("FrameOverTime %i {") ) handleMinMaxCurve(proxy.tsaFrameOverTime, fr);
        iteratorAdvanced = true; ++fr;
    }
    
    if ( fr.matchSequence("Renderer {") )
    {
        fr += 2;
        if ( fr.matchSequence("ShapeMode %w") )
        {
            proxy.renderShapeMode = fr[1].getStr();
            fr += 2;
        }
        
        if ( fr.matchSequence("SortMode %w") )
        {
            proxy.renderSortMode = fr[1].getStr();
            fr += 2;
        }
        
        if ( fr.matchSequence("Attributes %f %f %f %f") )
        {
            for (int i=0; i<4; ++i) fr[1+i].getFloat( proxy.renderAttributes[i] );
            iteratorAdvanced = true; fr += 5;
        }
        
        if ( fr.matchSequence("Material %i {") )
        {
            int numMaterials = 0; fr[1].getInt(numMaterials);
            proxy.renderTextures.resize( numMaterials );
            proxy.renderTilingOffsets.resize( numMaterials );
            fr += 3;
            
            for ( int i=0; i<numMaterials; ++i )
            {
                std::stringstream ss; ss << "Texture" << i << " %s %s";
                if ( fr.matchSequence(ss.str().c_str()) )
                {
                    std::string texName = fr[1].getStr(), texPath = fr[2].getStr();
                    proxy.renderTextures[i] = texPath;
                    fr += 3;
                }
                
                ss.str(""); ss << "TilingOffset" << i << " %f %f %f %f";
                if ( fr.matchSequence(ss.str().c_str()) )
                {
                    osg::Vec4 tilingOffset;
                    for ( int n=0; n<4; ++n ) fr[1 + n].getFloat( tilingOffset[n] );
                    proxy.renderTilingOffsets[i].set( tilingOffset[0], tilingOffset[1],
                                                      tilingOffset[2], tilingOffset[3] );
                    fr += 5;
                }
            }
            ++fr;
        }
        iteratorAdvanced = true; ++fr;
    }
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
