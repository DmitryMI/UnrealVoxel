// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoxelEngine : ModuleRules
{
	public VoxelEngine(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(new string[] { 
                "Core", 
                "CoreUObject", 
                "Engine", 
                "GeometryCore",
                "GeometryFramework",
                "Boost"
            }
        );

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.Add("VoxelEngine/Public/Noise");
        PublicIncludePaths.Add("VoxelEngine/Public/Query");
        PrivateIncludePaths.Add("VoxelEngine/Private/Query");
        PublicIncludePaths.Add("VoxelEngine/Public/DataStructures");
        PrivateIncludePaths.Add("VoxelEngine/Private/DataStructures");
        PublicIncludePaths.Add("VoxelEngine/Public/Navigation");
        PrivateIncludePaths.Add("VoxelEngine/Private/Navigation");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
