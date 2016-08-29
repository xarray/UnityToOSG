using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

public class BundleTimeOfDay : BundleComponent
{
    override public void Preprocess()
    {
        unityTimeOfDay = unityComponent as TimeOfDay;
    }

    override public void QueryResources()
    {
    }

    new public static void Reset()
    {
    }

    public override SceneComponent GetObjectData()
    {
        var sceneData = new SceneTimeOfDay();
        sceneData.type = "TimeOfDay";
        sceneData.timeOn = unityTimeOfDay.TimeOn;
        sceneData.timeOff = unityTimeOfDay.TimeOff;
        return sceneData;
    }

    TimeOfDay unityTimeOfDay;
}

}
