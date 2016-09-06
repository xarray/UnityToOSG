using System;
using System.IO;
using System.IO.Compression;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class BundleTexture : BundleResource
{
    public static int GetSuggestedUnit( string propName )
    {
        int suggestedUnit = -1;
        if ( propName=="_MainTex" ) suggestedUnit = 0;
        //else if ( propName=="_LightMap" ) suggestedUnit = 1;
        else if ( propName=="_BumpMap" ) suggestedUnit = 2;
        else if ( propName=="_Illum" ) suggestedUnit = 3;
        else if ( propName=="_Specular" ) suggestedUnit = 4;
        //else if ( propName=="_Cube" ) suggestedUnit = 5;
        else Debug.LogWarning("Unassiagned texture property: " + propName);
        return suggestedUnit;
    }
    
    private BundleTexture( Texture texture, string propName )
    {
        this.unityTexture = texture as Texture2D;
        allTextures[texture] = this;
        name = propName + ": " + texture.name;
    }

    void preprocess()
    {
        //Debug.Log("preprocess - " + unityTexture);
    }

    void process()
    {
        //Debug.Log("process - " + unityTexture);
        if ( unityTexture!=null )
        {
            path = AssetDatabase.GetAssetPath(unityTexture);
            uniqueID = unityTexture.GetInstanceID();
        }
        
        /*
        TextureImporter textureImporter = AssetImporter.GetAtPath(path) as TextureImporter;
        if ( textureImporter.isReadable==false )
        {
            textureImporter.isReadable = true;
            AssetDatabase.ImportAsset( path );
        }

        Texture2D ntexture = new Texture2D(unityTexture.width, unityTexture.height, TextureFormat.ARGB32, false);
        ntexture.SetPixels32( unityTexture.GetPixels32() );
        ntexture.Apply();

        var bytes = ntexture.EncodeToPNG();
        base64PNGLength = bytes.Length;
        base64PNG =  System.Convert.ToBase64String(bytes, 0, bytes.Length);
        UnityEngine.Object.DestroyImmediate(ntexture);
        */
    }

    void postprocess()
    {
        //Debug.Log("postprocess - " + unityTexture);
    }

    public static BundleTexture RegisterTexture( Texture texture, string propName )
    {
        if ( allTextures.ContainsKey(texture) )
            return allTextures[texture];
        return new BundleTexture(texture, propName);
    }

    new public static void Preprocess()
    {
        foreach ( var texture in allTextures.Values )
        {
            texture.preprocess();
        }
    }

    new public static void Process()
    {
        foreach ( var texture in allTextures.Values )
        {
            texture.process();
        }
    }

    new public static void PostProcess()
    {
        foreach ( var texture in allTextures.Values )
        {
            texture.postprocess();
        }
    }

    new public static void Reset()
    {
        allTextures = new Dictionary<Texture, BundleTexture>();
    }

    public new SceneTexture GetObjectData()
    {
        var sceneData = new SceneTexture();
        sceneData.uniqueID = uniqueID;
        sceneData.name = name;
        sceneData.path = path;
        //sceneData.base64PNG = base64PNG;
        //sceneData.base64PNGLength = base64PNGLength;
        return sceneData;
    }

    public static List<SceneTexture> GenerateObjectList()
    {
        List<SceneTexture> textures = new List<SceneTexture>();
        foreach ( var texture in allTextures.Values )
            textures.Add( texture.GetObjectData() );
        return textures;
    }

    Texture2D unityTexture;
    public new string name;
    public string path;
    public string base64PNG;
    public int uniqueID;
    public int base64PNGLength;
    public static Dictionary<Texture, BundleTexture> allTextures;
}


}
