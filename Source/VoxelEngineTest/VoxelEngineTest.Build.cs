// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoxelEngineTest : ModuleRules
{
	public VoxelEngineTest(ReadOnlyTargetRules Target) : base(Target)
	{	
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd", "AutomationController", "Boost" });
        PrivateDependencyModuleNames.Add("FunctionalTesting");
		
        PrivateDependencyModuleNames.AddRange(new string[] { "VoxelEngine" });

        PublicIncludePaths.Add("VoxelEngineTest/Private/DataStructures");
    }
}
