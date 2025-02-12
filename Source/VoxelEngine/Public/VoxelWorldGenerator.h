// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VoxelWorldGenerator.generated.h"

class AVoxelWorld;

UCLASS()
class VOXELENGINE_API UVoxelWorldGenerator : public UObject
{
	GENERATED_BODY()
	
public:
	DECLARE_DYNAMIC_DELEGATE(FVoxelWorlGenerationFinished);

	virtual FIntVector2 GetWantedWorldSizeVoxels() const;
	virtual void GenerateWorld(AVoxelWorld* VoxelWorld, const FVoxelWorlGenerationFinished& Callback);
};
