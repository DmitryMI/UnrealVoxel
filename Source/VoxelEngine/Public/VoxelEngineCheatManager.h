// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "VoxelEngineCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class VOXELENGINE_API UVoxelEngineCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Exec)
	void DrawChunkWireframes(bool bEnabled);

	UFUNCTION(Exec)
	void DrawChunkWireframe(int32 ChunkX, int32 ChunkY, bool bEnabled);

	UFUNCTION(Exec)
	void RegenerateChunkMeshes();
};
