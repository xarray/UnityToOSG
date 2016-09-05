#include <osg/ComputeBoundsVisitor>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

static const char* commonFragCode = {
    "uniform sampler2D mainTexture;\n"
    "uniform sampler2D lightTexture;\n"
    "uniform sampler2D normalTexture;\n"
    "uniform sampler2D specularTexture;\n"
    "uniform sampler2D emissionTexture;\n"
    
    "void main() {\n"
    "    vec4 color = texture2D(mainTexture, gl_TexCoord[0].st);\n"
    "    vec4 light = texture2D(lightTexture, gl_TexCoord[1].st);\n"
    "    gl_FragColor = color * light;\n"
    "}\n"
};

osg::Texture* createFallbackTexture( const osg::Vec4ub& color )
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE );
    *(osg::Vec4ub*)image->data() = color;
    
    osg::ref_ptr<osg::Texture2D> fallbackTexture = new osg::Texture2D( image.get() );
    fallbackTexture->setWrap( osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT );
    fallbackTexture->setWrap( osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT );
    fallbackTexture->setFilter( osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST );
    fallbackTexture->setFilter( osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST );
    return fallbackTexture.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osgViewer::Viewer viewer;
    
    // Build the scene graph
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( osgDB::readNodeFiles(arguments) );
    
    osg::StateSet* ss = root->getOrCreateStateSet();
    ss->setTextureAttributeAndModes( 0, createFallbackTexture(osg::Vec4ub(255, 255, 255, 255)) );  // main texture
    ss->setTextureAttributeAndModes( 1, createFallbackTexture(osg::Vec4ub(255, 255, 255, 255)) );  // light map
    ss->setTextureAttributeAndModes( 2, createFallbackTexture(osg::Vec4ub(0, 0, 0, 0)) );  // bump map
    ss->setTextureAttributeAndModes( 3, createFallbackTexture(osg::Vec4ub(0, 0, 0, 255)) );  // specular map
    ss->setTextureAttributeAndModes( 4, createFallbackTexture(osg::Vec4ub(0, 0, 0, 0)) );  // illumince map
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, commonFragCode) );
    ss->setAttributeAndModes( program.get() );
    ss->addUniform( new osg::Uniform("mainTexture", (int)0) );
    ss->addUniform( new osg::Uniform("lightTexture", (int)1) );
    ss->addUniform( new osg::Uniform("normalTexture", (int)2) );
    ss->addUniform( new osg::Uniform("specularTexture", (int)3) );
    ss->addUniform( new osg::Uniform("emissionTexture", (int)4) );
    
    // Start the viewer
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
