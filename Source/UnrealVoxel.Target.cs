// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class UnrealVoxelTarget : TargetRules
{
	public UnrealVoxelTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("UnrealVoxel");
        ExtraModuleNames.Add("VoxelEngine");

        if (Type == TargetType.Editor)
        {
            ExtraModuleNames.Add("VoxelEngineEditor");
        }
    }
}
