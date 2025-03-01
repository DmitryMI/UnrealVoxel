#pragma once

#include "CoreMinimal.h"
#include "VoxelNavAgentData.generated.h"

USTRUCT(BlueprintType)
struct FVoxelNavAgentData
{
	GENERATED_BODY();

	FVector AgentSize;
	double JumpHeight;
};