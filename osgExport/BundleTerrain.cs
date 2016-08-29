using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class BundleTerrain : BundleComponent
{
    override public void Preprocess()
    {
        terrain = unityComponent as Terrain;
        terrainData = terrain.terrainData;
    }

    override public void Process()
    {
        heightmapHeight = terrainData.heightmapHeight;
        heightmapWidth = terrainData.heightmapWidth;
        heightmapResolution = terrainData.heightmapResolution;
        heightmapScale = terrainData.heightmapScale;
        size = terrainData.size;

        float[,] heights = terrainData.GetHeights(0, 0, heightmapWidth, heightmapHeight);
        float[] arrayHeight = new float[heightmapWidth * heightmapHeight];
        for ( int y=0; y<heightmapHeight; y++ )
        {
            for ( int x=0; x<heightmapWidth; x++ )
                arrayHeight[y *  heightmapWidth + x] = heights[x, y];
        }
        
        alphamapWidth = terrainData.alphamapWidth;
        alphamapHeight = terrainData.alphamapHeight;
        alphamapLayers = terrainData.alphamapLayers;

        float [,,] alphamaps = terrainData.GetAlphamaps(0,0, alphamapWidth, alphamapHeight);
        float[] arrayAlpha = new float[alphamapWidth * alphamapHeight * alphamapLayers];
        for ( int i=0; i<alphamapLayers; i++ )
        {
            for ( int y=0; y<alphamapHeight; y++ )
            {
                for ( int x=0; x<alphamapWidth; x++ )
                    arrayAlpha[i * (alphamapHeight * alphamapWidth) + (y * alphamapWidth) + x] = alphamaps[x, y, i];
            }
        }

        var byteHeightArray = new byte[arrayHeight.Length * 4];
        Buffer.BlockCopy(arrayHeight, 0, byteHeightArray, 0, byteHeightArray.Length);
        base64HeightLength = byteHeightArray.Length;
        base64Height =  System.Convert.ToBase64String(byteHeightArray, 0, byteHeightArray.Length);

        var byteAlphaArray = new byte[arrayAlpha.Length * 4];
        Buffer.BlockCopy(arrayAlpha, 0, byteAlphaArray, 0, byteAlphaArray.Length);
        base64AlphaLength = byteAlphaArray.Length;
        base64Alpha =  System.Convert.ToBase64String(byteAlphaArray, 0, byteAlphaArray.Length);
    }

    override public void QueryResources()
    {
    }

    new public static void Reset()
    {
    }

    public override SceneComponent GetObjectData()
    {
        var sceneData = new SceneTerrain();
        sceneData.type = "Terrain";
        sceneData.heightmapHeight = heightmapHeight;
        sceneData.heightmapWidth = heightmapWidth;
        sceneData.heightmapResolution = heightmapResolution;
        sceneData.heightmapScale = heightmapScale;
        sceneData.size = size;
        sceneData.alphamapWidth = alphamapWidth;
        sceneData.alphamapHeight = alphamapHeight;
        sceneData.alphamapLayers = alphamapLayers;
        
        sceneData.heightmapTexture = new SceneTexture();
        sceneData.heightmapTexture.base64PNG = base64Height;
        sceneData.heightmapTexture.base64PNGLength = base64HeightLength;
        
        sceneData.alphamapTexture = new SceneTexture();
        sceneData.alphamapTexture.base64PNG = base64Alpha;
        sceneData.alphamapTexture.base64PNGLength = base64AlphaLength;
        return sceneData;
    }

    public Terrain terrain;
    private TerrainData terrainData;

    int heightmapHeight;
    int heightmapWidth;
    int heightmapResolution;
    Vector3 heightmapScale;
    Vector3 size;

    int alphamapWidth;
    int alphamapHeight;
    int alphamapLayers;

    public string base64Height;
    public int base64HeightLength;

    public string base64Alpha;
    public int base64AlphaLength;
}

}
