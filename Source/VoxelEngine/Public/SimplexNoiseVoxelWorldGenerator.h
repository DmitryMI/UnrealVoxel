// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "SimplexNoiseVoxelWorldGenerator.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class VOXELENGINE_API USimplexNoiseVoxelWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()
	
public:
	FIntVector2 GetWantedWorldSizeVoxels() const override;
	void GenerateWorld(AVoxelWorld* VoxelWorld, const FVoxelWorlGenerationFinished& Callback) override;

private:
	UPROPERTY(EditDefaultsOnly)
	FIntVector2 WorldSize = FIntVector2(128, 128);

	UPROPERTY(EditDefaultsOnly)
	int32 TerrainAverageHeight = 8;

	UPROPERTY(EditDefaultsOnly)
	int32 HeightAmplitude = 8;

	UPROPERTY(EditDefaultsOnly)
	float NoiseScale = 0.01;

	UPROPERTY(EditDefaultsOnly)
	int32 GenerationSeed = 1337;
};
