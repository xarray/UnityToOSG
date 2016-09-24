#include <osg/ComputeBoundsVisitor>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/TerrainManipulator>
#include <osgUtil/TangentSpaceGenerator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include "user_data_classes.h"

#define GET_TEXTURE_ID( name, var ) \
    GLuint var = 0; itr = textureIDMap.find(name); \
    if ( itr!=textureIDMap.end() ) var = itr->second;
SPK::SPK_ID createSmoke( const nwCore::SparkDrawable* spark, const nwCore::SparkDrawable::TextureIDMap& textureIDMap,
                         int screenWidth, int screenHeight )
{
    nwCore::SparkDrawable::TextureIDMap::const_iterator itr;
    GET_TEXTURE_ID( "smoke", textureParticle );
    
    SPK::GL::GLQuadRenderer* particleRenderer = SPK::GL::GLQuadRenderer::create();
    particleRenderer->setTexturingMode( SPK::TEXTURE_2D );
    particleRenderer->setAtlasDimensions( 2, 2 );
    particleRenderer->setTexture( textureParticle );
    particleRenderer->setTextureBlending( GL_MODULATE );
    particleRenderer->setScale( 0.05f, 0.05f );
    particleRenderer->setBlending( SPK::BLENDING_ADD );
    particleRenderer->enableRenderingHint( SPK::DEPTH_WRITE, false );
    
    // Model
    SPK::Model* particleModel = SPK::Model::create(
        SPK::FLAG_SIZE | SPK::FLAG_ALPHA | SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE,
        SPK::FLAG_SIZE | SPK::FLAG_ALPHA,
        SPK::FLAG_SIZE | SPK::FLAG_TEXTURE_INDEX | SPK::FLAG_ANGLE );
    particleModel->setParam( SPK::PARAM_SIZE, 0.5f, 1.0f, 10.0f, 20.0f );
    particleModel->setParam( SPK::PARAM_ALPHA, 1.0f, 0.0f );
    particleModel->setParam( SPK::PARAM_ANGLE, 0.0f, 2.0f * osg::PI );
    particleModel->setParam( SPK::PARAM_TEXTURE_INDEX, 0.0f, 4.0f );
    particleModel->setLifeTime( 2.0f, 5.0f );
    
    // Emitter
    SPK::SphericEmitter* particleEmitter = SPK::SphericEmitter::create(
        SPK::Vector3D(-1.0f, 0.0f, 0.0f), 0.0f, 0.1f * osg::PI );
    particleEmitter->setZone( SPK::Point::create(SPK::Vector3D(0.0f, 0.015f, 0.0f)) );
    particleEmitter->setFlow( 250.0 );
    particleEmitter->setForce( 1.5f, 1.5f );
    
    // Group
    SPK::Group* particleGroup = SPK::Group::create( particleModel, 500 );
    particleGroup->addEmitter( particleEmitter );
    particleGroup->setRenderer( particleRenderer );
    particleGroup->setGravity( SPK::Vector3D(0.0f, 0.0f, 0.05f) );
    particleGroup->enableAABBComputing( true );
    
    SPK::System* particleSystem = SPK::System::create();
    particleSystem->addGroup( particleGroup );
    particleSystem->enableAABBComputing( true );
    return particleSystem->getSPKID();
}

class UserDataFinder : public osg::NodeVisitor
{
public:
    UserDataFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    
    virtual void apply( osg::Node& node )
    {
        applyStateSet( node.getStateSet() );
        traverse( node );
    }

    virtual void apply( osg::Geode& node )
    {
        ParticleDataProxy* pd = dynamic_cast<ParticleDataProxy*>(&node);
        if ( pd && node.getNumParents()>0 )
        {
            particleDataMap[pd] = node.getParent(0);
            traverse( node );
            return;
        }
        
        for ( unsigned int i=0; i<node.getNumDrawables(); ++i )
        {
            osg::Geometry* geom = node.getDrawable(i)->asGeometry();
            applyStateSet( node.getDrawable(i)->getStateSet() );
            if ( !geom ) continue;
            
            TerrainDataProxy* td = dynamic_cast<TerrainDataProxy*>(geom);
            if ( !td )
            {
                osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
                tsg->generate( geom );
                if ( !geom->getVertexAttribArray(6) && tsg->getTangentArray() )
                    geom->setVertexAttribArray( 6, tsg->getTangentArray(), osg::Array::BIND_PER_VERTEX );
            }
            else
                terrainDataMap[td] = &node;
        }
        applyStateSet( node.getStateSet() );
        traverse( node );
    }

    void applyStateSet( osg::StateSet* ss )
    {
        ShaderDataProxy* sd = dynamic_cast<ShaderDataProxy*>(
            ss ? ss->getAttribute(osg::StateAttribute::PROGRAM) : NULL);
        if ( sd ) shaderDataMap[sd] = ss;
    }
    
    ShaderDataProxyMap shaderDataMap;
    TerrainDataProxyMap terrainDataMap;
    ParticleDataProxyMap particleDataMap;
};

osg::Texture* createFallbackTexture( const osg::Vec4ub& color )
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE );
    *(osg::Vec4ub*)image->data() = color;
    
    osg::ref_ptr<osg::Texture2D> fallbackTexture = new osg::Texture2D( image.get() );
    fallbackTexture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
    fallbackTexture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
    fallbackTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    fallbackTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    return fallbackTexture.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osgViewer::Viewer viewer;
    
    std::string databasePath = osgDB::getFilePath(arguments[0]);
    if ( databasePath.empty() ) databasePath = ".";
    
    LightUniforms sceneLight;
    sceneLight.color = new osg::Uniform("lightColor", osg::Vec3(1.0f, 1.0f, 1.0f));
    sceneLight.direction = new osg::Uniform("lightDirection", osg::Vec3(-0.5f, 0.0f, 0.5f));
    
    // Build the scene graph
    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    root->setMatrix( osg::Matrix::scale(-1.0, 1.0, 1.0) * osg::Matrix::rotate(osg::PI_2, osg::X_AXIS) );  // FIXME
    root->addChild( osgDB::readNodeFiles(arguments) );
    
    osg::StateSet* ss = root->getOrCreateStateSet();
    ss->setTextureAttributeAndModes( 0, createFallbackTexture(osg::Vec4ub(255, 255, 255, 255)) );  // main texture
    ss->setTextureAttributeAndModes( 1, createFallbackTexture(osg::Vec4ub(255, 255, 255, 255)) );  // light map
    ss->setTextureAttributeAndModes( 2, createFallbackTexture(osg::Vec4ub(0, 0, 0, 0)) );  // bump map
    ss->setTextureAttributeAndModes( 3, createFallbackTexture(osg::Vec4ub(0, 0, 0, 0)) );  // illumince map
    ss->setTextureAttributeAndModes( 4, createFallbackTexture(osg::Vec4ub(0, 0, 0, 255)) );  // specular map
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( osgDB::readShaderFile(osg::Shader::VERTEX, databasePath + "/shaders/Default.vert") );
    program->addShader( osgDB::readShaderFile(osg::Shader::FRAGMENT, databasePath + "/shaders/Default.frag") );
    program->addBindAttribLocation( "tangent", 6 );
    ss->setAttributeAndModes( program.get() );
    ss->addUniform( new osg::Uniform("mainTexture", (int)0) );
    ss->addUniform( new osg::Uniform("lightTexture", (int)1) );
    ss->addUniform( new osg::Uniform("normalTexture", (int)2) );
    ss->addUniform( new osg::Uniform("specularTexture", (int)3) );
    ss->addUniform( new osg::Uniform("emissionTexture", (int)4) );
    ss->addUniform( sceneLight.color.get() );
    ss->addUniform( sceneLight.direction.get() );
    
    // Traverse the scene and handle all user-data classes
    UserDataFinder udf;
    root->accept( udf );
    applyUserShaders( udf.shaderDataMap, databasePath );
    applyUserTerrains( udf.terrainDataMap, databasePath );
    
    osg::ref_ptr<nwCore::SparkUpdatingHandler> sparkHandler = new nwCore::SparkUpdatingHandler;
    applyUserParticles( udf.particleDataMap, sparkHandler.get(), databasePath );
    viewer.addEventHandler( sparkHandler.get() );
    
    viewer.getCamera()->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );
    root->setInitialBound( osg::BoundingSphere(osg::Vec3(), 10.0f) );
    
    // Start the viewer
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.setSceneData( root.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
