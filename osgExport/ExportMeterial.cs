using System;
using System.IO;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using UnityEditor.SceneManagement;

namespace nwTools
{

public class MaterialExporter
{
    public static string ExportStateSetAttr( bool isTransparent, string spaces )
    {
        string osgData = spaces + "  DataVariance STATIC\n";
        if ( isTransparent )
        {
            osgData += spaces + "  rendering_hint TRANSPARENT_BIN\n"
                     + spaces + "  renderBinMode USE\n"
                     + spaces + "  binNumber 10\n"
                     + spaces + "  binName DepthSortedBin\n";
        }
        else
        {
            osgData += spaces + "  rendering_hint DEFAULT_BIN\n"
                     + spaces + "  renderBinMode INHERIT\n";
        }
        return osgData;
    }
    
    public static string ExportTextureAttr( ref SceneTexture texture, string spaces )
    {
        string osgData = spaces + "  DataVariance STATIC\n"
                       + spaces + "  name \"" + texture.name + "\"\n"
                       + spaces + "  file \"" + texture.path + "\"\n"
                       + spaces + "  wrap_s REPEAT\n"
                       + spaces + "  wrap_t REPEAT\n"
                       + spaces + "  wrap_r REPEAT\n"
                       + spaces + "  min_filter LINEAR_MIPMAP_LINEAR\n"
                       + spaces + "  mag_filter LINEAR\n"
                       + spaces + "  maxAnisotropy 1\n"
                       + spaces + "  borderColor 0 0 0 0\n"
                       + spaces + "  borderWidth 0\n"
                       + spaces + "  useHardwareMipMapGeneration TRUE\n"
                       + spaces + "  unRefImageDataAfterApply TRUE\n"
                       + spaces + "  internalFormatMode USE_IMAGE_DATA_FORMAT\n"
                       + spaces + "  resizeNonPowerOfTwo TRUE\n";  // TODO: tiling and offset
        return osgData;
    }
    
    public static string ExportStateSet( ref SceneData sceneData, ref SceneMeshRenderer smr, string spaces )
    {
        string osgData = spaces + "StateSet {\n" + ExportStateSetAttr(false, spaces);
        for ( int i=0; i<smr.materials.Length; ++i )
        {
            SceneMaterial material = sceneData.resources.GetMaterial(smr.materials[i]);
            if ( material.textureIDs==null ) continue;
            
            for ( int j=0; j<material.textureIDs.Length; ++j )
            {
                int texID = material.textureIDs[j], unit = material.textureUnits[j];
                SceneTexture texture = sceneData.resources.GetTexture(texID, false);
                if ( texture==null || unit<0 ) continue;
                
                osgData += spaces + "  textureUnit " + unit + " {\n"
                         + spaces + "    GL_TEXTURE_2D ON\n";
                if ( sharedTextureNames.ContainsKey(texID) )
                {
                    osgData += spaces + "    Use " + sharedTextureNames[texID] + "\n";
                }
                else
                {
                    sharedTextureNames[texID] = "Texture_" + texID;
                    osgData += spaces + "    Texture2D {\n"
                             + spaces + "      UniqueID Texture_" + texID + "\n"
                             + ExportTextureAttr(ref texture, spaces + "    ")
                             + spaces + "    }\n";
                }
                osgData += spaces + "  }\n";
            }
            
            // Save shader data for use
            osgData += spaces + "  nwTools::ShaderData {\n"
                     + spaces + "    ShaderName \"" + material.shader + "\"\n";
            if ( material.shaderKeywords!=null )
            {
                osgData += spaces + "    Keywords ";
                for ( int k=0; k<material.shaderKeywords.Length; ++k )
                {
                    osgData += material.shaderKeywords[k]
                             + ((k < material.shaderKeywords.Length-1) ? " " : "\n");
                }
            }
            osgData += spaces + "  }\n";
        }
        
        if ( smr.lightmapIndex>=0 )
        {
            SceneTexture texture = sceneData.resources.lightmaps[smr.lightmapIndex];
            if ( texture!=null )
            {
                osgData += spaces + "  textureUnit 1 {\n"  // FIXME: always 1?
                         + spaces + "    GL_TEXTURE_2D ON\n";
                osgData += spaces + "    Texture2D {\n"
                         + ExportTextureAttr(ref texture, spaces + "    ")
                         + spaces + "    }\n"
                         + spaces + "  }\n";
            }
        }
        osgData += spaces + "}\n";
        return osgData;
    }
    
    public static void Reset()
    {
        sharedTextureNames = new Dictionary<int, string>();
    }
    
    public static Dictionary<int, string> sharedTextureNames;
}

}
