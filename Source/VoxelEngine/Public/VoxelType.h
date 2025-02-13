#pragma once

#include "CoreMinimal.h"
#include "VoxelType.generated.h"

using VoxelType = uint8;

USTRUCT(BlueprintType)
struct FVoxelType
{
    GENERATED_BODY()

public:
    constexpr static VoxelType EmptyVoxelType = 0;

    static FVoxelType EmptyVoxel()
    {
        return FVoxelType(EmptyVoxelType);
    }

    VoxelType VoxelTypeId;

    operator VoxelType() const
    {
        return VoxelTypeId;
    }

    operator int32() const
    {
        return VoxelTypeId;
    }

    FVoxelType()
    {
        VoxelTypeId = EmptyVoxelType;
    }

    FVoxelType(VoxelType Value)
    {
        VoxelTypeId = Value;
    }

    bool IsEmptyVoxelType() const
    {
        return VoxelTypeId == EmptyVoxelType;
    }
};
