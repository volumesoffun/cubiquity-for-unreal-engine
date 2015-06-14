// Copyright 2014 Volumes of Fun. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Cubiquity : ModuleRules
{
    private string ModulePath
    {
        get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "C:/Program Files/Cubiquity-2015-06-14")); } //Edit this line to match where Cubiquity is installed
    }

    private string ThirdPartyLibraryPath
    {
        //Change "Release" to something else for different build types
        get { return Path.Combine(ThirdPartyPath, "bin"); }
    }

    private string ThirdPartyIncludePath
    {
        get { return Path.Combine(ThirdPartyPath, "include"); }
    }
    
    public Cubiquity(TargetInfo Target)
	{
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = true;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore", "ShaderCore" });

        LoadCubiquity(Target);
	}

    public bool LoadCubiquity(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        if((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";

            //string remotePath = @"http://www.volumesoffun.com/downloads/Cubiquity/Cubiquity-2015-06-14.zip";
            //System.IO.Compression.ZipFile.ExtractToDirectory(remotePath, extractPath);

            //Copy the Cubiquity DLL into the binaries directory locally
            FileInfo file = new FileInfo(Path.Combine(ThirdPartyLibraryPath, "CubiquityC.dll"));
            FileInfo destFile = new FileInfo(Path.Combine(ModulePath, "..", "..", "Binaries", PlatformString, "CubiquityC.dll"));
            if (destFile.Exists)
            {
                if (file.LastWriteTime > destFile.LastWriteTime)
                {
                    file.CopyTo(destFile.FullName, true);
                }
            }
            else
            {
                file.CopyTo(destFile.FullName, true);
            }

            //Make sure we can link against the .lib file
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyLibraryPath, "CubiquityC.lib"));
        }

        if(isLibrarySupported)
        {
            // Include path
            PublicIncludePaths.Add(ThirdPartyIncludePath);
        }

        return isLibrarySupported;
    }
}
