using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using UnityEngine;
using UnityEditor;
using UnityEditor.SceneManagement;

namespace nwTools
{

public class SceneExporter : ScriptableObject
{
    [MenuItem ("nwTools/Export All")]
    public static void DoExportAll() { ExportScene(false); }

    [MenuItem ("nwTools/Export Selected")]
    public static void DoExportSelected() { ExportScene(true); }
    
    static void reset()
    {
        BundleResource.Reset();
        BundleComponent.Reset();
        BundleScene.Reset();
        BundleGameObject.Reset();
        BundleComponent.RegisterStandardComponents();
        
        MeshExporter.Reset();
        MaterialExporter.Reset();
    }

    public static SceneData GenerateSceneData( bool onlySelected )
    {
        // reset the exporter in case there was an error, Unity doesn't cleanly load/unload editor assemblies
        reset();
        
        BundleScene.sceneName = Path.GetFileNameWithoutExtension(EditorSceneManager.GetActiveScene().name);
        BundleScene scene = BundleScene.TraverseScene(onlySelected);
        scene.Preprocess();
        scene.Process();
        scene.PostProcess();

        SceneData sceneData = scene.GetSceneData() as SceneData;
        reset();
        return sceneData;
    }
    
    public static void ExportScene( bool onlySelected )
    {
        var defaultFileName = Path.GetFileNameWithoutExtension(EditorSceneManager.GetActiveScene().name) + ".osg";
        var path = EditorUtility.SaveFilePanel( "Export Scene to Bundler", "", defaultFileName, "osg" );
        if ( path.Length!=0 )
        {
            EditorUtility.DisplayProgressBar( "Scene Bundler", "Start exporting...", 0.0f );
            SceneData sceneData = GenerateSceneData(onlySelected);
            
            float numHierarchy = (float)sceneData.hierarchy.Count, numDone = 0.0f;
            string osgData = ExportHeaderOSG( ref sceneData );
            foreach ( SceneGameObject obj in sceneData.hierarchy )
            {
                osgData += ExportHierarchy( ref sceneData, obj, 2, numDone, numHierarchy ); numDone += 1.0f;
                EditorUtility.DisplayProgressBar( "Scene Bundler", "Exporting hierarchy...", numDone/numHierarchy );
            }
            osgData += "}\n";
            
            EditorUtility.DisplayProgressBar( "Scene Bundler", "Writing to " + path, 1.0f );
            System.IO.File.WriteAllText( path, osgData );
            
            EditorUtility.ClearProgressBar();
            EditorUtility.DisplayDialog( "Scene Bundler", "Export Successful", "OK" );
        }
    }
    
    private static string ExportCommonAttr( string name, string spaces )
    {
        string osgData = spaces + "  DataVariance STATIC\n"
                       + spaces + "  name \"" + name + "\"\n"
                       + spaces + "  nodeMask 0xffffffff\n"
                       + spaces + "  cullingActive TRUE\n";
        return osgData;
    }
    
    private static string ExportHeaderOSG( ref SceneData sceneData )
    {
        string osgData = "Group {\n" + ExportCommonAttr(sceneData.name, "")
                       + "  num_children " + sceneData.hierarchy.Count + "\n";
        return osgData;
    }
    
    private static string ExportHierarchy( ref SceneData sceneData, SceneGameObject gameObj,
                                           int indent, float progressStart, float progressAll )
    {
        int needGlobalNodeType = -1;
        if ( gameObj.components.Count<=0 ) return "";
        
        string osgData = "", osgSubData = "", spaces = "";
        for ( int i=0; i<indent; ++i ) spaces += " ";
        
        // Check the main component type as the node type
        SceneComponent mainComponent = gameObj.components[0];
        if ( mainComponent.type=="Transform" )
        {
            SceneTransform st = (SceneTransform)mainComponent;
            osgData = spaces + "MatrixTransform {\n"
                    + spaces + "  referenceFrame RELATIVE\n"
                    + spaces + "  Matrix {\n";
            needGlobalNodeType = 0;
            
            // FIXME: hould convert left-handed to right-handed coordinates
            Matrix4x4 m = Matrix4x4.TRS(st.localPosition, st.localRotation, st.localScale);
            osgData += spaces + "    " + m[0, 0] + " " + m[1, 0] + " " + m[2, 0] + " " + m[3, 0] + "\n"
                     + spaces + "    " + m[0, 1] + " " + m[1, 1] + " " + m[2, 1] + " " + m[3, 1] + "\n"
                     + spaces + "    " + m[0, 2] + " " + m[1, 2] + " " + m[2, 2] + " " + m[3, 2] + "\n"
                     + spaces + "    " + m[0, 3] + " " + m[1, 3] + " " + m[2, 3] + " " + m[3, 3] + "\n"
                     + spaces + "  }\n";
        }
        else
            Debug.LogWarning( "[UnityToSceneBundle] Unknown main component type: " + mainComponent.type );
        
        if ( needGlobalNodeType<0 ) osgData = spaces + "Node {\n";
        osgData += ExportCommonAttr(gameObj.name, spaces)
                 + spaces + "  num_children ";
        
        // Traverse all components to add them to main component type
        string subSpaces = spaces + "    ";
        int numChildren = gameObj.children.Count;
        for ( int i=1; i<gameObj.components.Count; ++i )
        {
            SceneComponent component = gameObj.components[i];
            if ( component.type=="Light" )
            {
                SceneLight sl = (SceneLight)component;
                osgSubData += spaces + "  nwTools::LightData {\n"
                            + subSpaces + "Type " + sl.lightType + "\n"
                            + subSpaces + "Color " + sl.color.r + " " + sl.color.g + " " + sl.color.b + "\n"
                            + subSpaces + "Range " + sl.range + "\n"
                            + subSpaces + "Realtime " + (sl.realtime ? 1 : 0) + " " + (sl.castsShadows ? 1 : 0) + "\n"
                            + spaces + "  }\n";
                numChildren++;
            }
            else if ( component.type=="Camera" )
            {
                SceneCamera sc = (SceneCamera)component;
                osgSubData += spaces + "  nwTools::CameraData {\n"
                            // TODO
                            + spaces + "  }\n";
                numChildren++;
            }
            else if ( component.type=="BoxCollider" )
            {
                //SceneBoxCollider sbc = (SceneBoxCollider)component;
                // TODO
            }
            else if ( component.type=="MeshCollider" )
            {
                //SceneMeshCollider smc = (SceneMeshCollider)component;
                // TODO
            }
            else if ( component.type=="MeshRenderer" )
            {
                SceneMeshRenderer smr = (SceneMeshRenderer)component;
                osgSubData += spaces + "  Geode {\n"
                            + ExportCommonAttr(smr.type, spaces + "  ")
                            + subSpaces + "num_drawables 1\n";
                
                SceneMesh mesh = sceneData.resources.GetMesh(smr.mesh);
                osgSubData += subSpaces + "Geometry {\n"
                            + MeshExporter.ExportGeometry(ref sceneData, ref smr, ref mesh, subSpaces + "  ")
                            + subSpaces + "}\n";
                osgSubData += spaces + "  }\n";
                numChildren++;
            }
            else
                Debug.LogWarning( "[UnityToSceneBundle] Unknown sub-component type: " + component.type );
        }
        osgData += numChildren + "\n" + osgSubData;
        
        // Traverse all child objects
        float numHierarchy = (float)gameObj.children.Count, numDone = 0.0f;
        foreach ( SceneGameObject childObj in gameObj.children )
        {
            osgData += ExportHierarchy( ref sceneData, childObj, indent + 2, 0.0f, 0.0f ); numDone += 1.0f;
            if ( progressAll>0.0f )
            {
                float progress = (progressStart + numDone / numHierarchy) / progressAll;
                EditorUtility.DisplayProgressBar( "Scene Bundler", "Exporting hierarchy...", progress );
            }
        }
        osgData += spaces + "}\n";
        return osgData;
    }
}

}
