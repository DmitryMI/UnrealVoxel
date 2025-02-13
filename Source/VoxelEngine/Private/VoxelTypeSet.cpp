// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelTypeSet.h"
#include "VoxelEngine/VoxelEngine.h"

const TArray<UVoxelData*>& UVoxelTypeSet::GetVoxelTypes() const
{
	return VoxelTypes;
}

UVoxelData* UVoxelTypeSet::GetVoxelDataByName(const FName& VoxelName) const
{
	for (UVoxelData* Voxel : VoxelTypes)
	{
		if (Voxel->VoxelName == VoxelName)
		{
			return Voxel;
		}
	}

	UE_LOG(LogVoxelEngine, Error, TEXT("Voxel Type with name %s was not found!"), *VoxelName.ToString());
	return nullptr;
}

UVoxelData* UVoxelTypeSet::GetVoxelDataByTypeId(int32 VoxelTypeId) const
{
	return GetVoxelDataByType(VoxelTypeId);
}

FVoxelType UVoxelTypeSet::GetVoxelTypeByName(const FName& VoxelName) const
{
	for (int I = 0; I < VoxelTypes.Num(); I++)
	{
		if (VoxelTypes[I]->VoxelName == VoxelName)
		{
			return I + 1;
		}
	}

	return FVoxelType::EmptyVoxelType;
}

UVoxelData* UVoxelTypeSet::GetVoxelDataByType(FVoxelType VoxelType) const
{
	check(VoxelType.VoxelTypeId != FVoxelType::EmptyVoxelType);
	check(VoxelTypes.Num() > VoxelType.VoxelTypeId - 1);
	return VoxelTypes[VoxelType.VoxelTypeId - 1];
}

#if WITH_EDITOR
TArray<UVoxelData*>& UVoxelTypeSet::GetVoxelTypesMutable()
{
	return VoxelTypes;
}
#endif
