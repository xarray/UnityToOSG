using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class BundleParticleSystem : BundleComponent
{
    override public void Preprocess()
    {
        unityParticle = unityComponent as ParticleSystem;
    }

    new public static void Reset()
    {
    }

    public override SceneComponent GetObjectData()
    {
        var sceneData = new SceneParticleSystem();
        sceneData.type = "ParticleSystem";
        if ( unityParticle!=null )
        {
            //
        }
        return sceneData;
    }

    ParticleSystem unityParticle;
}

}
