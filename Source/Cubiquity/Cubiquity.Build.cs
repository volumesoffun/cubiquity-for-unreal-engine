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
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/CubiquityC")); } //Edit this line to match where Cubiquity is installed
    }

    private string ThirdPartyLibraryPath
    {
        get { return Path.Combine(ThirdPartyPath, "Lib"); }
    }

    private string ThirdPartyIncludePath
    {
        get { return Path.Combine(ThirdPartyPath, "Src"); }
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

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            isLibrarySupported = true;

            string ConfigurationDirString;
            if (Target.Configuration == UnrealTargetConfiguration.Debug && BuildConfiguration.bDebugBuildsActuallyUseDebugCRT)
            {
                ConfigurationDirString = "Debug";
            }
            else if (Target.Configuration == UnrealTargetConfiguration.Shipping)
            {
                ConfigurationDirString = "Release";
            }
            else
            {
                ConfigurationDirString = "RelWithDebInfo";
            }

            //string remotePath = @"http://www.volumesoffun.com/downloads/Cubiquity/Cubiquity-2015-06-14.zip";
            //System.IO.Compression.ZipFile.ExtractToDirectory(remotePath, extractPath);

            //Copy the Cubiquity DLL into the binaries directory locally
            FileInfo file = new FileInfo(Path.Combine(ThirdPartyLibraryPath, ConfigurationDirString, "CubiquityC.dll"));
            DirectoryInfo destDir = new DirectoryInfo(Path.Combine(ModulePath, "..", "..", "Binaries/Win64"));
            destDir.Create();
            FileInfo destFile = new FileInfo(Path.Combine(destDir.ToString(), "CubiquityC.dll"));
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
            PublicLibraryPaths.Add(Path.Combine(ThirdPartyLibraryPath, ConfigurationDirString));
            PublicAdditionalLibraries.Add("CubiquityC.lib");
        }

        if(isLibrarySupported)
        {
            // Include path
            PublicIncludePaths.Add(ThirdPartyIncludePath);
        }

        return isLibrarySupported;
    }
}
