using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace nwTools
{

static class ExportError
{
    public static void FatalError( string message )
    {
        EditorUtility.ClearProgressBar();
        EditorUtility.DisplayDialog( "Error", message, "Ok" );
        throw new Exception(message);
    }
}

}
