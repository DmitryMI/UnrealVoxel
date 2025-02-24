// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoxelEngineTest : ModuleRules
{
	public VoxelEngineTest(ReadOnlyTargetRules Target) : base(Target)
	{	
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd", "AutomationController" });
        PrivateDependencyModuleNames.Add("FunctionalTesting");
		
        PrivateDependencyModuleNames.AddRange(new string[] { "VoxelEngine" });

        PublicIncludePaths.Add("VoxelEngineTest/Private/DataStructures");
    }
}
