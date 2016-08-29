using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class BundleTransform : BundleComponent
{
    override public void Preprocess()
    {
        unityTransform = unityComponent as Transform;
    }

    new public static void Reset()
    {
    }

    public override SceneComponent GetObjectData()
    {
        var sceneData = new SceneTransform();
        sceneData.type = "Transform";
        sceneData.localPosition = unityTransform.localPosition;
        sceneData.localRotation = unityTransform.localRotation;
        sceneData.localScale = unityTransform.localScale;
        return sceneData;
    }

    Transform unityTransform;
}

}
