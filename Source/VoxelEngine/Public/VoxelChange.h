// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelType.h"

UENUM()
enum class EVoxelChangeRenderPriority
{
	// Voxel Change must happen inside method call
	Immidiate,
	// Voxel Change must happen during the same frame as the method call
	SameFrame,
	// Voxel Change may happen any time
	AnyTime
};

UENUM()
enum class EVoxelChangeExpectationMismatch
{
	Reject = 0,
	Overwrite = 1
};

UENUM()
enum class EVoxelChangeResult
{
	// Voxel change was executed and renderer was notified
	Executed = 0,
	// Voxel change is invalid, do not retry the same request.
	Rejected = 2
};

/**
 * 
 */
struct VOXELENGINE_API FVoxelChange
{
	FIntVector Coordinate{ 0, 0, 0 };
	EVoxelChangeRenderPriority Priority = EVoxelChangeRenderPriority::AnyTime;
	EVoxelChangeExpectationMismatch ExpectationMismatch = EVoxelChangeExpectationMismatch::Reject;
	VoxelType ExpectedVoxelType = EmptyVoxelType;
	VoxelType ChangeToVoxelType = EmptyVoxelType;
};
