// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelType.h"
#include <atomic>

/**
 * 
 */
class VOXELENGINE_API Voxel
{
public:
	std::atomic<VoxelType> VoxelTypeId;
	Voxel(VoxelType VoxelTypeId) : VoxelTypeId(VoxelTypeId)
	{
		
	}

	Voxel(const Voxel& Voxel) : VoxelTypeId(Voxel.VoxelTypeId.load())
	{

	}
};
