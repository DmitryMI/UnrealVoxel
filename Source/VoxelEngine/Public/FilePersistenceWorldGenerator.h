// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "FilePersistenceWorldGenerator.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class VOXELENGINE_API UFilePersistenceWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	bool LoadWorld(const FString& Path);

	UFUNCTION(BlueprintCallable)
	bool SaveWorld(AVoxelWorld* VoxelWorld, const FString& Path);

	FIntVector2 GetWantedWorldSizeVoxels() const override;
	void GenerateWorld(AVoxelWorld* VoxelWorld, const FVoxelWorlGenerationFinished& Callback) override;
	
private:
	FString LoadedFilePath;

	bool ArePathsEqual(const FString& Path1, const FString& Path2) const;
};
