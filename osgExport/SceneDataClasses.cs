using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class SceneTexture
{
    public string name;
    public string path;
    public string base64PNG;
    public int base64PNGLength;
    public int uniqueID;
}

public class SceneShader
{
    public string name;
    public int renderQueue;
}

public class SceneMaterial
{
    public string name;
    public string shader;
    public int[] textureIDs;
    public int[] textureUnits;
    public Vector4[] textureTilingOffsets;
    public int passCount;
    public int renderQueue;
    public string[] shaderKeywords;
    public Color color;
}

public class SceneBoneWeight
{
    public int[] indexes = new int[4];
    public float[] weights = new float[4];
}

public class SceneMesh
{
    public string name;
    public int subMeshCount;
    public int[][] triangles;
    public int vertexCount;
    public Vector3[] vertexPositions;
    public Vector2[] vertexUV;
    public Vector2[] vertexUV2;
    public Color[] vertexColors;
    public Vector3[] vertexNormals;
    public Vector4[] vertexTangents;
    public Matrix4x4[] bindPoses;
    public SceneBoneWeight[] boneWeights;
    public SceneTransform[] bones;
    public string rootBone;
}

public class SceneGameObject
{
    public string name;
    public List<SceneComponent> components;
    public List<SceneGameObject> children;
    
    public T GetComponent<T>() where T: SceneComponent
    {
        foreach ( var component in components )
        {
            if ( component.GetType()==typeof(T) )
                return (T)component;
        }
        return null;
    }
}

public class SceneComponent
{
    public string type;
}

public class SceneTransform : SceneComponent
{
    public Vector3 localPosition;
    public Quaternion localRotation;
    public Vector3 localScale;
    public String name;
    public String parentName;
}

public class SceneTimeOfDay : SceneComponent
{
    public float timeOn;
    public float timeOff;
}

public class SceneCamera : SceneComponent
{
    // TODO
}

public class SceneLight : SceneComponent
{
    public Color color;
    public float range;
    public string lightType;
    public bool castsShadows;
    public bool realtime;
}

public class SceneBoxCollider : SceneComponent
{
    public Vector3 center;
    public Vector3 size;
}

public class SceneMeshCollider : SceneComponent
{
    public SceneMesh mesh;
}

public class SceneRigidBody : SceneComponent
{
    public float mass;
}

public class SceneMeshRenderer : SceneComponent
{
    public string mesh;
    public bool enabled;
    public bool castShadows;
    public bool receiveShadows;
    public int lightmapIndex;
    public Vector4 lightmapTilingOffset;
    public string[] materials;
}

public class SceneSkinnedMeshRenderer : SceneMeshRenderer
{
    public string rootBone;
}

public class SceneParticleSystem : SceneComponent
{
    // Basic attributes
    public float duration, playingSpeed;
    public int maxParticles;
    public bool isLooping, isAutoStarted;
    public Vector3 gravity, rotation;
    public Vector4 startAttributes;  // [life, size, speed, delay]
    public Color startColor;
    public string[] enabledModules;
    
    // Emission module  // TODO: bursts, curve data
    public Vector4[] emissionRate;  // [time, value, inTangent, outTangent]
    public string emissionType;
    
    // Texture Sheet Animation module  // TODO: curve data
    public int tsaCycleCount;
    public Vector2 tsaNumTiles;
    public Vector4[] tsaFrameOverTime;  // [time, value, inTangent, outTangent]
    public string tsaAnimationType;
    
    // Renderer module  // TODO: render as mesh
    public Vector4 renderAttributes;  // [minSize, maxSize, normalDir, sortFudge]
    public string renderShapeMode, renderSortMode;
    public string renderMaterial;
}

public class SceneKeyframe
{
    public Vector3 pos;
    public Vector3 scale;
    public Quaternion rot;
    public float time;
}

public class SceneAnimationNode
{
    public string name;
    public SceneKeyframe[] keyframes;
}

public class SceneAnimationClip
{
    public string name;
    public SceneAnimationNode[] nodes;
}

public class SceneAnimation : SceneComponent
{
    public SceneAnimationClip[] clips;
}

public class SceneTerrain : SceneComponent
{
    public int heightmapHeight;
    public int heightmapWidth;
    public Vector3 size;

    public int alphamapWidth;
    public int alphamapHeight;
    public int alphamapLayers;

    public SceneTexture heightmapTexture;
    public SceneTexture alphamapTexture;
    
    public int[] textureIDs;
    public Vector4[] textureTilingOffsets;
    public int lightmapIndex;
    public Vector4 lightmapTilingOffset;
}

public class SceneResources
{
    public List<SceneTexture> textures;
    public List<SceneTexture> lightmaps;
    public List<SceneShader> shaders;
    public List<SceneMaterial> materials;
    public List<SceneMesh> meshes;
    
    public SceneTexture GetTexture( int texID, bool asLightmap )
    {
        if ( asLightmap )
        {
            foreach ( var texture in lightmaps )
            {
                if ( texture.uniqueID==texID )
                    return texture;
            }
        }
        
        foreach ( var texture in textures )
        {
            if ( texture.uniqueID==texID )
                return texture;
        }
        return null;
    }

    public SceneTexture GetTextureByName( string name, bool asLightmap )
    {
        if ( asLightmap )
        {
            foreach ( var texture in lightmaps )
            {
                if ( texture.name==name )
                    return texture;
            }
        }
        
        foreach ( var texture in textures )
        {
            if ( texture.name==name )
                return texture;
        }
        return null;
    }
    
    public SceneMaterial GetMaterial( string name )
    {
        foreach ( var material in materials )
        {
            if ( material.name==name )
                return material;
        }
        return null;
    }

    public SceneMesh GetMesh( string name )
    {
        foreach ( var mesh in meshes )
        {
            if ( mesh.name==name )
                return mesh;
        }
        return null;
    }
}

public class SceneData
{
    public string name;
    public SceneResources resources;
    public List<SceneGameObject> hierarchy;
}

}
