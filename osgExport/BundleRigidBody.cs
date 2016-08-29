using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class BundleRigidBody : BundleComponent
{
    override public void Preprocess()
    {
        unityRigidBody = unityComponent as Rigidbody;
    }

    override public void QueryResources()
    {
    }

    new public static void Reset()
    {
    }

    public override SceneComponent GetObjectData()
    {
        var sceneData = new SceneRigidBody();
        sceneData.type = "RigidBody";
        sceneData.mass = unityRigidBody.mass;
        return sceneData;
    }

    Rigidbody unityRigidBody;
}

}
