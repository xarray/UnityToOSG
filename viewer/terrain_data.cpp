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
                va->push_back( osg::Vec3(float(y) * proxy->size[2] / float(proxy->heightMapSize[1] - 1),
                                         proxy->heightMap[index] * proxy->size[1],
                                         float(x) * proxy->size[0] / float(proxy->heightMapSize[0] - 1)) );
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
        
        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setVertexArray( va.get() );
        //geom->setTexCoordArray( 0, ta.get() );
        geom->addPrimitiveSet( de.get() );
        
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
    
    // TODO
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
