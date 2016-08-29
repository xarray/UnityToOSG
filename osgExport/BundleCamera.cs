using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class BundleCamera : BundleComponent
{
    override public void Preprocess()
    {
        unityCamera = unityComponent as Camera;
    }
    
    new public static void Reset()
    {
    }

    public override SceneComponent GetObjectData()
    {
        var sceneData = new SceneCamera();
        sceneData.type = "Camera";
        return sceneData;
    }
    
    Camera unityCamera;
}

}
