// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CubiquityC : ModuleRules
{
    public CubiquityC(TargetInfo Target)
	{
        Type = ModuleType.External;

        PublicIncludePaths.Add("CubiquityC/Src/");
        PrivateIncludePaths.Add("CubiquityC/Src/Dependancies/");

        string CubiquityCLibPath = "CubiquityC/Lib/";

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			CubiquityCLibPath += "Win64/VS" + WindowsPlatform.GetVisualStudioCompilerVersionName();
			PublicLibraryPaths.Add(CubiquityCLibPath);

			if (Target.Configuration == UnrealTargetConfiguration.Debug && BuildConfiguration.bDebugBuildsActuallyUseDebugCRT)
			{
				PublicAdditionalLibraries.Add("CubiquityCD_64.lib");
			}
			else
			{
				PublicAdditionalLibraries.Add("CubiquityC_64.lib");
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Win32)
		{
			CubiquityCLibPath += "Win32/VS" + WindowsPlatform.GetVisualStudioCompilerVersionName();
			PublicLibraryPaths.Add(CubiquityCLibPath);
			if (Target.Configuration == UnrealTargetConfiguration.Debug && BuildConfiguration.bDebugBuildsActuallyUseDebugCRT)
			{
				PublicAdditionalLibraries.Add("CubiquityCD.lib");
			}
			else
			{
				PublicAdditionalLibraries.Add("CubiquityC.lib");
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string Postfix = (Target.Configuration == UnrealTargetConfiguration.Debug && BuildConfiguration.bDebugBuildsActuallyUseDebugCRT) ? "d" : "";
			PublicAdditionalLibraries.Add(CubiquityCLibPath + "Mac/libCubiquityC" + Postfix + ".a");
		}
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string Postfix = (Target.Configuration == UnrealTargetConfiguration.Debug && BuildConfiguration.bDebugBuildsActuallyUseDebugCRT) ? "d" : "";
            PublicAdditionalLibraries.Add(CubiquityCLibPath + "Linux/" + Target.Architecture + "/libCubiquityC" + Postfix + ".a");
        }
	}
}
