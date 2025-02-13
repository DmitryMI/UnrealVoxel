// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelData.h"
#include "VoxelType.h"
#include "VoxelTypeSet.generated.h"

/**
 * 
 */
UCLASS()
class VOXELENGINE_API UVoxelTypeSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditDefaultsOnly)
	TArray<UVoxelData*> VoxelTypes;

public:
	UFUNCTION(BlueprintCallable)
	const TArray<UVoxelData*>& GetVoxelTypes() const;

	UFUNCTION(BlueprintCallable)
	UVoxelData* GetVoxelDataByName(const FName& VoxelName) const;

	UFUNCTION(BlueprintCallable)
	UVoxelData* GetVoxelDataByTypeId(int32 VoxelTypeId) const;

	FVoxelType GetVoxelTypeByName(const FName& VoxelName) const;

	UVoxelData* GetVoxelDataByType(FVoxelType VoxelType) const;

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable)
	TArray<UVoxelData*>& GetVoxelTypesMutable();
#endif
};
