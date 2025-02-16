// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelType.h"
#include "VoxelChange.generated.h"

UENUM(BlueprintType)
enum class EVoxelChangeRenderPriority : uint8
{
	// Voxel Change must happen inside method call
	Immidiate,
	// Voxel Change must happen during the same frame as the method call
	SameFrame,
	// Voxel Change may happen any time
	AnyTime
};

UENUM(BlueprintType)
enum class EVoxelChangeExpectationMismatch : uint8
{
	Cancel = 0,
	Overwrite = 1
};

UENUM(BlueprintType)
enum class EVoxelChangeResult : uint8
{
	// Voxel change was executed and renderer was notified
	Executed = 0,
	// Request was valid, but voxel type in memory did not match the expected type. Retry the request with updated expectation.
	ExpectationMismatch = 1,
	// Voxel change is invalid, do not retry the same request.
	Rejected = 2
};


struct VOXELENGINE_API FVoxelChange
{
	FIntVector Coordinate{ 0, 0, 0 };
	EVoxelChangeRenderPriority Priority = EVoxelChangeRenderPriority::AnyTime;
	EVoxelChangeExpectationMismatch ExpectationMismatch = EVoxelChangeExpectationMismatch::Cancel;

	VoxelType ExpectedVoxelType = EmptyVoxelType;
	VoxelType ChangeToVoxelType = EmptyVoxelType;

	FVoxelChange()
	{

	}

	FVoxelChange(const FIntVector& Coord, VoxelType Expected, VoxelType Desired):
		Coordinate(Coord),
		Priority(EVoxelChangeRenderPriority::AnyTime),
		ExpectationMismatch(EVoxelChangeExpectationMismatch::Cancel),
		ExpectedVoxelType(Expected),
		ChangeToVoxelType(Desired)
	{

	}
};
