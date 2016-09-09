# UnityToOSG
Convert Unity scene to OpenSceneGraph format

## Usage
Copy osgExport and osgExportHelpers to your Unity project folder\Assets
You will find a new "nwTools" menu at the top and use menu items to export .osg files!
Note that the texture paths are always relative to Unity project folder, so you may have to redirect them

## TODO List
1. Support multiple texture maps (done)
2. Support lightmaps and lightprobes (partly)
3. Use specified viewer (with a default shader) to show scene (done)
4. Support light and camera data outputs (partly)
5. Make use of dynamic lights from Forward+ pass
6. Support game object and camera animations
7. Support character data and animation outputs
8. Play kinds of animations in viewer
9. Support mesh collider data outputs (partly)
10. Load and use colliders in physics engine integrations
11. Support particle animations
12. Support Unity legacy shaders and reimplement them in OSG (partly)
13. Support terrains and plants (partly)
14. Support Unity 5.x standard PBR shader
