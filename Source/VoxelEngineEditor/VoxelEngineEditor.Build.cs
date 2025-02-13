// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoxelEngineEditor : ModuleRules
{
	public VoxelEngineEditor(ReadOnlyTargetRules Target) : base(Target)
	{	
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "AssetRegistry" });
        PrivateDependencyModuleNames.AddRange(new string[] { "VoxelEngine" });

    }
}
