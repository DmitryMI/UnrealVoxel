// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorldGenerator.h"
#include "VoxelWorld.h"

FIntVector2 UVoxelWorldGenerator::GetWantedWorldSizeVoxels() const
{
	return FIntVector2(0, 0);
}

void UVoxelWorldGenerator::GenerateWorld(AVoxelWorld* VoxelWorld, const FVoxelWorlGenerationFinished& Callback)
{
	Callback.ExecuteIfBound();
}
