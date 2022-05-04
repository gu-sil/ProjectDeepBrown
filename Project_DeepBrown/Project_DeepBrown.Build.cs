// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_DeepBrown : ModuleRules
{
	public Project_DeepBrown(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivateDependencyModuleNames.AddRange(new string[] { "ApexDestruction" });
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
