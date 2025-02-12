// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class UnrealVoxelEditorTarget : TargetRules
{
	public UnrealVoxelEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("UnrealVoxel");
        ExtraModuleNames.Add("VoxelEngine");
        ExtraModuleNames.Add("VoxelEngineEditor");
    }
}
