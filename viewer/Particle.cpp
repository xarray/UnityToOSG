#include "Particle.h"
#include <algorithm>

#include <osgDB/ReadFile>
#include <osgViewer/View>

using namespace SPK;
using namespace nwCore;

/* SparkDrawable::DeferredSystemHandler */

void SparkDrawable::DeferredSystemHandler::update( osg::NodeVisitor* nv, osg::Drawable* drawable )
{
    SparkDrawable* spark = static_cast<SparkDrawable*>(drawable);
    if ( !spark || !spark->isValid() ) return;
    
    if ( _newSystemsToAdd.size()>0 )
    {
        for ( unsigned int i=0; i<_newSystemsToAdd.size(); ++i )
        {
            const PosAndRotate& pr = _newSystemsToAdd[i];
            spark->createParticleSystem( pr.position, pr.rotationAxis, pr.rotationAngle );
        }
        _newSystemsToAdd.clear();
    }
}

/* SparkDrawable */

SparkDrawable::SparkDrawable()
:   _baseSystemCreator(NULL), _baseSystemID(SPK::NO_ID), _protoSystem(NULL),
    _lastTime(-1.0), _sortParticles(false), _useProtoSystem(true),
    _autoUpdateBound(true), _updatingActive(true), _drawingActive(true), _dirty(true),
    _compileAddedImage( false )
{
    _activeContextID = 0;
    setUpdateCallback( new DeferredSystemHandler );
    setSupportsDisplayList( false );
    setDataVariance( osg::Object::DYNAMIC );
}
    
SparkDrawable::SparkDrawable( const SparkDrawable& copy,const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop),
    _textureObjMap(copy._textureObjMap), _particleSystems(copy._particleSystems),
    _baseSystemCreator(copy._baseSystemCreator), _baseSystemID(copy._baseSystemID),
    _protoSystem(copy._protoSystem), _activeContextID(copy._activeContextID),
    _lastTime(copy._lastTime), _sortParticles(copy._sortParticles),
    _useProtoSystem(copy._useProtoSystem), _autoUpdateBound(copy._autoUpdateBound),
    _updatingActive(copy._updatingActive), _drawingActive(copy._drawingActive),
    _dirty(copy._dirty)
{}

SparkDrawable::~SparkDrawable()
{
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        destroyParticleSystem( *itr, false );
    }
    if ( _protoSystem )
        destroyParticleSystem( _protoSystem, false );
}

unsigned int SparkDrawable::getNumParticles() const
{
    unsigned int count = 0;
    if ( _useProtoSystem && _protoSystem )
        count += _protoSystem->getNbParticles();
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        count += (*itr)->getNbParticles();
    }
    return count;
}

void SparkDrawable::setGlobalTransformMatrix( const osg::Matrix& matrix, bool useOffset )
{
    osg::Vec3d trans = matrix.getTrans();
    osg::Quat quat = matrix.getRotate();
    osg::Vec3d axis; double angle = 0.0f;
    quat.getRotate( angle, axis );
    
    SPK::Vector3D pos(trans.x(), trans.y(), trans.z());
    SPK::Vector3D rot(axis.x(), axis.y(), axis.z());
    if ( _useProtoSystem && _protoSystem )
        setTransformMatrix( _protoSystem, pos, rot, angle, useOffset );
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        setTransformMatrix( *itr, pos, rot, angle, useOffset );
    }
}

void SparkDrawable::setTransformMatrix( SPK::System* system, const SPK::Vector3D& pos, const SPK::Vector3D& rot,
                                        float angle, bool useOffset )
{
    if ( useOffset )
    {
        system->setTransformPosition( pos + system->getLocalTransformPos() );
        system->setTransformOrientation( rot, angle );  // FIXME: how to get rotation offset?
    }
    else
    {
        system->setTransformPosition( pos );
        system->setTransformOrientation( rot, angle );
    }
    system->updateTransform();
}

unsigned int SparkDrawable::addParticleSystem( const osg::Vec3& p, const osg::Quat& r )
{
    DeferredSystemHandler* updater = dynamic_cast<DeferredSystemHandler*>( getUpdateCallback() );
    if ( updater )
    {
        osg::Vec3 axis; double angle = 0.0f;
        r.getRotate( angle, axis );
        
        DeferredSystemHandler::PosAndRotate pr;
        pr.position = SPK::Vector3D( p.x(), p.y(), p.z() );
        pr.rotationAxis = SPK::Vector3D( axis.x(), axis.y(), axis.z() );
        pr.rotationAngle = angle;
        updater->_newSystemsToAdd.push_back( pr );
    }
    return _particleSystems.size() + updater->_newSystemsToAdd.size() - 1;
}

void SparkDrawable::addImage( const std::string& name, osg::Image* image, GLuint type, GLuint clamp )
{
    if ( image )
    {
        ImageAttribute attr;
        attr.image = image;
        attr.type = type;
        attr.clamp = clamp;
        _textureObjMap[name] = attr;
    }
    else
    {
        OSG_NOTICE << "SparkDrawable::addImage: empty image file for " << name << std::endl;
    }
}

bool SparkDrawable::update( double currentTime, const osg::Vec3d& eye )
{
    bool active = false;
    if ( _dirty ) return false;  // wait until base system initialized
    if ( !_updatingActive )
    {
        _lastTime = -1.0f;
        return false;
    }
    
    if ( _lastTime>0.0 )
    {
        if ( _sortParticles )
            std::sort( _particleSystems.begin(), _particleSystems.end(), SortParticlesOperator(eye) );
        
        double deltaTime = currentTime - _lastTime;
        SPK::Vector3D eyePos(eye.x(), eye.y(), eye.z());
        if ( _useProtoSystem && _protoSystem )
        {
            _protoSystem->setCameraPosition( eyePos );
            active = _protoSystem->update(deltaTime);
        }
        
        ParticleSystemList::iterator itr = _particleSystems.begin();
        while( itr!=_particleSystems.end() )
        {
            (*itr)->setCameraPosition( eyePos );
            if ( !(*itr)->update(deltaTime) )
            {
                destroyParticleSystem( *itr, false );
                itr = _particleSystems.erase( itr );
            }
            else
            {
                active = true;
                ++itr;
            }
        }
        
        if ( _autoUpdateBound )
            dirtyBound();  // Update the particle bound for near/far computing and culling
    }
    else
        active = true;
    
    _lastTime = currentTime;
    return active;
}

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
osg::BoundingSphere SparkDrawable::computeBound() const
#else
osg::BoundingBox SparkDrawable::computeBound() const
#endif
{
    osg::BoundingBox bb;
    SPK::Vector3D min, max;
    if ( _useProtoSystem && _protoSystem )
    {
        if ( _protoSystem->isAABBComputingEnabled() )
        {
            _protoSystem->computeAABB();
            min = _protoSystem->getAABBMin(); bb.expandBy( osg::Vec3(min.x, min.y, min.z) );
            max = _protoSystem->getAABBMax(); bb.expandBy( osg::Vec3(max.x, max.y, max.z) );
        }
    }
    
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        SPK::System* system = *itr;
        if ( system->isAABBComputingEnabled() )
        {
            system->computeAABB();
            min = system->getAABBMin(); bb.expandBy( osg::Vec3(min.x, min.y, min.z) );
            max = system->getAABBMax(); bb.expandBy( osg::Vec3(max.x, max.y, max.z) );
        }
    }
    return bb;
}

void SparkDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getContextID();
    if ( _dirty )
    {
        if ( _baseSystemCreator )
        {
            /*TextureIDMap textureIDMap;
            for ( TextureObjMap::const_iterator itr=_textureObjMap.begin();
                  itr!=_textureObjMap.end(); ++itr )
            {
                const ImageAttribute& attr = itr->second;
                textureIDMap[itr->first] =
                    compileInternalTexture(attr.image.get(), attr.type, attr.clamp);
            }*/
            genTextureID();
            _baseSystemID = (*_baseSystemCreator)( this, _textureIDMap, 800, 600 );
            _protoSystem = SPK_Get( SPK::System, _baseSystemID );
        }
        
        _activeContextID = contextID;
        _dirty = false;
    }
    else if ( contextID!=_activeContextID || !_drawingActive )
        return;

    if ( _compileAddedImage )
    {
        genTextureID();
        _compileAddedImage = false;
    }
    
    osg::State* state = renderInfo.getState();
    state->disableAllVertexArrays();
    
    // Make sure the client unit and active unit are unified
    state->setClientActiveTextureUnit( 0 );
    state->setActiveTextureUnit( 0 );
    state->Normal( 0.0f, 1.0f, 0.0f );  // restore normal state
    
    SPK::GL::GLRenderer::saveGLStates();
    if ( _useProtoSystem && _protoSystem ) _protoSystem->render();
    for ( ParticleSystemList::const_iterator itr=_particleSystems.begin();
          itr!=_particleSystems.end(); ++itr )
    {
        (*itr)->render();
    }
    SPK::GL::GLRenderer::restoreGLStates();
}

SPK::System* SparkDrawable::createParticleSystem( const SPK::Vector3D& pos, const SPK::Vector3D& rot, float angle )
{
    SPK::System* system = SPK_Copy( SPK::System, _baseSystemID );
    if ( !system ) return NULL;
    else setTransformMatrix( system, pos, rot, angle );
    
    _particleSystems.push_back( system );
    return system;
}

GLuint SparkDrawable::compileInternalTexture( osg::Image* image, GLuint type, GLuint clamp ) const
{
    GLuint index;
    glGenTextures( 1, &index );
    glBindTexture( GL_TEXTURE_2D, index );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    
    unsigned int numCurrent = osg::Image::computeNumComponents( image->getPixelFormat() );
    unsigned int numRequired = osg::Image::computeNumComponents( type );
    if ( numCurrent!=numRequired && image->getDataType()==GL_UNSIGNED_BYTE )
        convertData( image, type, numCurrent, numRequired );
    
    /*if ( mipmap )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        gluBuild2DMipmaps( GL_TEXTURE_2D, type, image->s(), image->t(),
                           type, GL_UNSIGNED_BYTE, image->data() );
    }
    else*/
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D, 0, type, image->s(), image->t(), 0,
                      type, GL_UNSIGNED_BYTE, image->data() );
    }
    return index;
}

void SparkDrawable::convertData( osg::Image* image, GLuint type,
                                 unsigned int numCurrent, unsigned int numRequired ) const
{
    int newRowWidth = osg::Image::computeRowWidthInBytes(image->s(), type, GL_UNSIGNED_BYTE, 1);
    unsigned char* newData = new unsigned char[newRowWidth * image->t() * image->r()];
    
    for ( int t=0; t<image->t(); ++t )
    {
        unsigned char* source = image->data(0, t);
        unsigned char* dest = newData + t * newRowWidth;
        for ( int s=0; s<image->s(); ++s )
        {
            if ( numRequired==1 )  // RGB/RGBA -> ALPHA
            {
                *dest++ = *source;
                source += numCurrent;
            }
            else
            {
                OSG_WARN << image->getFileName() << ": no conversation from "
                         << numCurrent << " elements to " << numRequired << " elements" << std::endl;
            }
        }
    }
    image->setImage( image->s(), image->t(), image->r(), numRequired, type,
                     GL_UNSIGNED_BYTE, newData, osg::Image::USE_NEW_DELETE );
}

void SparkDrawable::genTextureID() const
{
    for ( TextureObjMap::const_iterator itr=_textureObjMap.begin();
        itr!=_textureObjMap.end(); ++itr )
    {
        const ImageAttribute& attr = itr->second;
        TextureIDMap::const_iterator itrID = _textureIDMap.find( itr->first );
        // if not exist in _textureIDMap
        // TODO:
        if ( itrID==_textureIDMap.end() )
        {
            _textureIDMap[itr->first] =
                compileInternalTexture(attr.image.get(), attr.type, attr.clamp);
        }
    }
}

void SparkDrawable::addImageLater( const std::string& name, const std::string& file, GLuint type/*=GL_RGB*/, GLuint clamp/*=GL_CLAMP */ )
{
    osg::Image* image = osgDB::readImageFile( file );
    addImage( name, image, type, clamp );
    _compileAddedImage = true;
}

GLuint SparkDrawable::getTextureID( const std::string& texName )
{
    TextureIDMap::const_iterator itr = _textureIDMap.find( texName );
    if ( itr!=_textureIDMap.end() )
        return itr->second;
    return 0;
}

std::string SparkDrawable::getTextureNameByID( GLuint texID )
{
    std::string texName;
    for ( TextureIDMap::const_iterator itr = _textureIDMap.begin();
        itr != _textureIDMap.end(); itr++ )
    {
        if ( itr->second == texID )
        {
            texName = itr->first;
            break;
        }
    }
    return texName;
}

/* CollisionGroup */

CollisionGroup::CollisionGroup( SPK::Model* model, size_t capacity )
:   SPK::Group(model, capacity), _canCollide(false)
{
    registerObject( this );
    setCustomUpdate( &CollisionGroup::checkParticleCollision );
    _result = new Result;
}

CollisionGroup::CollisionGroup( const CollisionGroup& copy )
:   SPK::Group(copy), _result(copy._result), _colliderCenter(copy._colliderCenter),
    _colliderDir(copy._colliderDir), _canCollide(copy._canCollide)
{
}

void CollisionGroup::setCollideData( const osg::Vec3& result, const osg::Vec3& dir, bool valid )
{
    _colliderCenter = result;
    _colliderDir = dir;
    _canCollide = valid;
    _result->_collidedNumber = 0;
}

bool CollisionGroup::isCollided() const
{
    bool c = (_result->_collidedNumber > 0);
    _result->_collidedNumber = 0; return c;
}

bool CollisionGroup::checkParticleCollision( SPK::Particle& p, float deltaTime )
{
    CollisionGroup* group = static_cast<CollisionGroup*>( p.getGroup() );
    if ( group && group->_canCollide )
    {
        osg::Vec3 diff = osg::Vec3(p.position().x, p.position().y, p.position().z)
                       - group->_colliderCenter;
        diff.normalize();
        if ( (diff * group->_colliderDir)>0.5 )
        {
            if ( !group->_result->_collidedNumber )
                group->_result->_lastCenter = group->_colliderCenter;
            group->_result->_collidedNumber++;
            return true;
        }
    }
    return false;
}

/* SparkUpdatingHandler */

void SparkUpdatingHandler::setTrackee( SparkDrawable* spark, osg::Transform* t )
{
    for (unsigned int i=0; i<_sparks.size(); ++i)
    {
        if ( _sparks[i]._spark==spark )
        {
            setTrackee(i, t);
            break;
        }
    }
}

bool SparkUpdatingHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
    if ( !view ) return false;
    
    osg::Camera* camera = view->getCamera();
    if ( !camera ) return false;
    
    double time = view->getFrameStamp()->getSimulationTime();
    if ( ea.getEventType()==osgGA::GUIEventAdapter::FRAME )
    {
        osg::Vec3d eye, center, up;
        camera->getViewMatrixAsLookAt( eye, center, up );
        
        for ( std::vector<SparkObject>::iterator itr=_sparks.begin(); itr!=_sparks.end(); ++itr )
        {
            SparkDrawable* spark = itr->_spark.get();
            if ( !spark ) continue;
            
            osg::Transform* trackee = itr->_trackee.get();
            if ( trackee )
            {
                if ( itr->_dirtyMatrix )
                {
                    itr->_transformMatrix = computeTransformMatrix( spark, trackee );
                    itr->_dirtyMatrix = false;
                }
                
                osg::Matrix matrix; trackee->computeLocalToWorldMatrix(matrix, NULL);
                spark->setGlobalTransformMatrix( matrix * itr->_transformMatrix );
            }
            spark->update( time, eye );
        }
    }
    return false;
}

osg::Matrix SparkUpdatingHandler::computeTransformMatrix( SparkDrawable* spark, osg::Transform* trackee )
{
    osg::Node* sparkGeode = (spark->getNumParents()>0 ? spark->getParent(0) : NULL);
    if ( !sparkGeode )
        return osg::Matrix::identity();
    else if ( !sparkGeode->getNumParents() || !trackee->getNumParents() )
        return osg::Matrix::identity();
    else if ( sparkGeode->getParent(0)==trackee->getParent(0) )
        return osg::Matrix::identity();
    
    // Collect the parent paths, ignoring the last one (the spark/trackee itself)
    osg::NodePath& sparkPath = sparkGeode->getParentalNodePaths()[0]; sparkPath.pop_back();
    osg::NodePath& trackeePath = trackee->getParentalNodePaths()[0];  trackeePath.pop_back();
    return computeLocalToWorld(trackeePath) * computeWorldToLocal(sparkPath);
}
