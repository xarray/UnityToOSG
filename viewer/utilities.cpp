#include <osgDB/ReadFile>
#include <osg/Image>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class ReaderWriterSTB : public osgDB::ReaderWriter
{
public:
    ReaderWriterSTB() { supportsExtension("psd", "Photoshop extension"); }
    virtual const char* className() const { return "Reader based on stb-image"; }
    
    virtual bool acceptsExtension( const std::string& ext ) const
    { return osgDB::equalCaseInsensitive(ext, "psd"); }
    
    virtual ReadResult readImage( const std::string& fileName,
                                  const osgDB::ReaderWriter::Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        
        int w = 0, h = 0, numComponents = 0;
        unsigned char* data = stbi_load( fileName.c_str(), &w, &h, &numComponents, 0 );
        if ( !data ) return ReadResult::ERROR_IN_READING_FILE;
        
        GLenum pixelFormat = GL_RGBA;
        switch ( numComponents )
        {
        case 1: pixelFormat = GL_LUMINANCE; break;
        case 2: pixelFormat = GL_LUMINANCE_ALPHA; break;
        case 3: pixelFormat = GL_RGB; break;
        default: break;
        }
        
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->setImage( w, h, 1, pixelFormat, pixelFormat, GL_UNSIGNED_BYTE,
                         data, osg::Image::USE_MALLOC_FREE );
        image->flipVertical();
        return image.get();
    }
};
REGISTER_OSGPLUGIN( psd, ReaderWriterSTB )
