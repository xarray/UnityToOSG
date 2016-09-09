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
    
    // Start the viewer
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setCameraManipulator( new osgGA::TerrainManipulator );
    viewer.setSceneData( root.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
