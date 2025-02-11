#pragma once

#include "CoreMinimal.h"
#include "VoxelType.generated.h"

UENUM(BlueprintType)
enum class EVoxelType : uint8
{
    Air UMETA(DisplayName = "Air"),
    Grass UMETA(DisplayName = "Grass"),
    Dirt UMETA(DisplayName = "Dirt"),
    Stone UMETA(DisplayName = "Stone")
};
