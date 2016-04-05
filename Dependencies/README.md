
[Forum Post](https://forums.unrealengine.com/showthread.php?29873-Cubiquity-for-UE4-Voxel-Terrain-Plugin&p=311218&viewfull=1#post311218)

    There's one or two files we need to edit before we start. 
    First is the Plugins/Cubiquity/Source/Cubiquity/Cubiquity.Build.cs file. 
    Edit the string in ThirdPartyPath to point to the place you unzipped the Cubiquity DLLs. 
    Secondly, edit Plugins/Cubiquity/Source/Cubiquity/Private/CubiquityColoredCubesVolume.cpp and CubiquityTerrainVolume.cpp. 
    Edit the string in the constructor of them to point to the place you unzipped the example VDB files.

    The last thing you will need to do is put CubiquityColoredCubesVertexFactory.usf in UnrealEngine/Engine/Shaders in the main Unreal Engine folder (not in your project).
