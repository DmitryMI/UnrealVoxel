// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelWorld.h"
#include "VoxelQueryUtils.generated.h"

UENUM()
enum class EVoxelLineTraceFilterMode : uint8
{
	DontCare = 0,
	Positive = 1,
	Negative = 2
};

USTRUCT(BlueprintType)
struct FVoxelLineTraceFilterParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVoxelLineTraceFilterMode Transparent = EVoxelLineTraceFilterMode::DontCare;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVoxelLineTraceFilterMode Traversible = EVoxelLineTraceFilterMode::DontCare;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MaxDistance = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeInitialVoxel = true;
};

UCLASS()
class VOXELENGINE_API UVoxelQueryUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Voxel Query Utils")
	static bool VoxelLineTraceFilterSingle(AVoxelWorld* VoxelWorld, const FVector& Start, const FVector& Direction, const FVoxelLineTraceFilterParams& Params, FIntVector& OutHitCoord);

private:
	static bool CheckIfVoxelSatisfiesFilter(AVoxelWorld* VoxelWorld,const FIntVector& Coord, const FVoxelLineTraceFilterParams& Params);

	static TArray<int> GetMinComponent(const FVector& Values, const TStaticArray<bool, 3>& ValidityFlags);
};
