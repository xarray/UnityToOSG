#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TangentSpaceGenerator>
#include <algorithm>
#include "user_data_classes.h"

void applyUserTerrains( TerrainDataProxyMap& tdMap, const std::string& dbPath )
{
    for ( TerrainDataProxyMap::iterator itr=tdMap.begin(); itr!=tdMap.end(); ++itr )
    {
        TerrainDataProxy* proxy = itr->first;
        osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec2Array> ta = new osg::Vec2Array;
        for ( short y=0; y<proxy->heightMapSize[1]; ++y )
        {
            for ( short x=0; x<proxy->heightMapSize[0]; ++x )
            {
                int index = int(x) + int(y) * int(proxy->heightMapSize[0]);
                float tx = float(y) / float(proxy->heightMapSize[1] - 1);
                float ty = float(x) / float(proxy->heightMapSize[0] - 1);
                va->push_back( osg::Vec3(
                    tx * proxy->size[2], proxy->heightMap[index] * proxy->size[1], ty * proxy->size[0]) );
                ta->push_back( osg::Vec2(ty, tx) );
            }
        }
        
        osg::ref_ptr<osg::DrawElementsUInt> de = new osg::DrawElementsUInt(GL_QUADS);
        for ( short y=1; y<proxy->heightMapSize[1]; ++y )
        {
            for ( short x=1; x<proxy->heightMapSize[0]; ++x )
            {
                de->push_back( int(x - 1) + int(y - 1) * int(proxy->heightMapSize[0]) );
                de->push_back( int(x) + int(y - 1) * int(proxy->heightMapSize[0]) );
                de->push_back( int(x) + int(y) * int(proxy->heightMapSize[0]) );
                de->push_back( int(x - 1) + int(y) * int(proxy->heightMapSize[0]) );
            }
        }
        
        // Create splat texture array
        osg::ref_ptr<osg::Texture2DArray> splatTexture = new osg::Texture2DArray;
        splatTexture->setTextureDepth( proxy->layers );
        splatTexture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
        splatTexture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
        splatTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR );
        splatTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        
        int splatW = 0, splatH = 0;
        for ( int i=0; i<proxy->layers; ++i )
        {
            osg::Image* image = osgDB::readImageFile(proxy->splatTextures[i]);
            splatTexture->setImage( i, image );
            
            if ( !image ) { OSG_NOTICE << "Bad splat file " << proxy->splatTextures[i] << std::endl; }
            else if ( !splatW || !splatH ) { splatW = image->s(); splatH = image->t(); }
            else if ( splatW!=image->s() || splatH!=image->t() ) image->scaleImage(splatW, splatH, 1);
        }
        
        // Create alpha texture
        unsigned int dataSize = proxy->alphaMaps.size() * sizeof(osg::Vec4ub);
        unsigned char* data = new unsigned char[dataSize];
        memcpy( data, (unsigned char*)&(proxy->alphaMaps[0]), dataSize );
        
        osg::ref_ptr<osg::Image> alphaImage = new osg::Image;
        alphaImage->setImage( proxy->alphaMapSize[0], proxy->alphaMapSize[1], 1, GL_RGBA, GL_RGBA,
                              GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE );
        
        osg::ref_ptr<osg::Texture2D> alphaTexture = new osg::Texture2D( alphaImage.get() );
        alphaTexture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
        alphaTexture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
        alphaTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR );
        alphaTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        
        // Create the terrain geometry
        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setVertexArray( va.get() );
        geom->setTexCoordArray( 0, ta.get() );
        geom->addPrimitiveSet( de.get() );
        
        osg::StateSet* ss = geom->getOrCreateStateSet();
        ss->setTextureAttribute( 0, splatTexture.get() );  // TODO: lightmap at unit 1?
        ss->setTextureAttributeAndModes( 2, alphaTexture.get() );
        
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader( osgDB::readShaderFile(osg::Shader::VERTEX, dbPath + "/shaders/Terrain.vert") );
        program->addShader( osgDB::readShaderFile(osg::Shader::FRAGMENT, dbPath + "/shaders/Terrain.frag") );
        ss->setAttributeAndModes( program.get() );
        ss->addUniform( new osg::Uniform("splatTexture", (int)0) );
        ss->addUniform( new osg::Uniform("lightTexture", (int)1) );
        ss->addUniform( new osg::Uniform("alphaTexture", (int)2) );
        
        osg::Uniform* splatTilingOffsetsUniform = ss->getOrCreateUniform(
            "splatTilingOffsets", osg::Uniform::FLOAT_VEC4, 4);
        for ( int i=0; i<proxy->layers; ++i )
            splatTilingOffsetsUniform->setElement( i, proxy->splatTilingOffsets[i] );
        
        // Add normal and tangent array for the geometry
        osgUtil::SmoothingVisitor smv;
        smv.smooth( *geom );
        
        /*osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
        tsg->generate( geom );
        if ( !geom->getVertexAttribArray(6) && tsg->getTangentArray() )
            geom->setVertexAttribArray( 6, tsg->getTangentArray(), osg::Array::BIND_PER_VERTEX );*/
        itr->second->removeDrawable( itr->first );
        itr->second->addDrawable( geom.get() );
    }
}

bool TerrainData_readLocalData( osg::Object& obj, osgDB::Input& fr )
{
    TerrainDataProxy& proxy = static_cast<TerrainDataProxy&>(obj);
    bool iteratorAdvanced = false;
    
    if ( fr.matchSequence("Size %f %f %f") )
    {
        fr[1].getFloat( proxy.size[0] );
        fr[2].getFloat( proxy.size[1] );
        fr[3].getFloat( proxy.size[2] );
        iteratorAdvanced = true; fr += 4;
    }
    
    if ( fr.matchSequence("HeightMap %i %i {") )
    {
        int w = 0, h = 0, index = 0;
        fr[1].getInt(w); fr[2].getInt(h);
        proxy.heightMapSize.set( w, h );
        proxy.heightMap.resize( w * h );
        
        int entry = fr[0].getNoNestedBrackets();
        fr += 4;
        while ( !fr.eof() && fr[0].getNoNestedBrackets()>entry )
        {
            fr[0].getFloat( proxy.heightMap[index] );
            ++fr; ++index;
        }
        iteratorAdvanced = true; ++fr;
    }
    
    proxy.layers = 0;
    if ( fr.matchSequence("AlphaMap %i %i %i {") )
    {
        int w = 0, h = 0;
        fr[1].getInt(w); fr[2].getInt(h); fr[3].getInt(proxy.layers);
        proxy.alphaMapSize.set( w, h );
        proxy.alphaMaps.resize( w * h );
        proxy.splatTextures.resize( proxy.layers );
        proxy.splatTilingOffsets.resize( proxy.layers );
        
        int entry0 = fr[0].getNoNestedBrackets();
        fr += 5;
        while ( !fr.eof() && fr[0].getNoNestedBrackets()>entry0 )
        {
            if ( fr.matchSequence("Layer %i {") )
            {
                int id = 0, index = 0;
                fr[1].getInt( id );
                
                int entry = fr[0].getNoNestedBrackets();
                fr += 3;
                if ( id<4 )
                {
                    while ( !fr.eof() && fr[0].getNoNestedBrackets()>entry )
                    {
                        float v = 0.0f; fr[0].getFloat(v);
                        proxy.alphaMaps[index][3 - id] = unsigned char(v * 255.0f);  // start from v.w
                        ++fr; ++index;
                    }
                }
                else
                {
                    OSG_WARN << "We only handle terrain with equal or less than 4 layers" << std::endl;
                    while ( !fr.eof() && fr[0].getNoNestedBrackets()>entry ) ++fr;
                }
                ++fr;
            }
        }
        iteratorAdvanced = true; ++fr;
    }
    
    for ( int i=0; i<proxy.layers; ++i )
    {
        std::stringstream ss; ss << "Splat" << i << " %s %s";
        if ( fr.matchSequence(ss.str().c_str()) )
        {
            std::string texName = fr[1].getStr(), texPath = fr[2].getStr();
            proxy.splatTextures[i] = texPath;
            iteratorAdvanced = true; fr += 3;
        }
        
        ss.str(""); ss << "SplatTilingOffset" << i << " %f %f %f %f";
        if ( fr.matchSequence(ss.str().c_str()) )
        {
            osg::Vec4 tilingOffset;
            for ( int n=0; n<4; ++n ) fr[1 + n].getFloat( tilingOffset[n] );
            proxy.splatTilingOffsets[i].set( tilingOffset[0], tilingOffset[1],
                                             tilingOffset[2], tilingOffset[3] );
            iteratorAdvanced = true; fr += 5;
        }
    }
    return iteratorAdvanced;
}

bool TerrainData_writeLocalData( const osg::Object&, osgDB::Output& )
{ return false; }

REGISTER_DOTOSGWRAPPER( TerrainData_Proxy )
(
    new TerrainDataProxy,
    "nwTools::Terrain",
    "Object Drawable Geometry nwTools::Terrain",
    TerrainData_readLocalData, TerrainData_writeLocalData
);
