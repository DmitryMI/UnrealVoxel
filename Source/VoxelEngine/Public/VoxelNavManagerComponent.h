// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelChange.h"
#include "VoxelEngine/Public/Navigation/NavNode.h"
#include "VoxelEngine/Public/DataStructures/RStarTree.h"
#include "VoxelNavManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOXELENGINE_API UVoxelNavManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	DECLARE_DYNAMIC_DELEGATE(FVoxelNavGenerationFinished);

	UVoxelNavManagerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GenerateNavData(const FVoxelNavGenerationFinished& Callback);
	void ChangeVoxel(const FVoxelChange& VoxelChange);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	bool bDebugDrawNavVolumes = true;

	UPROPERTY(EditDefaultsOnly)
	int32 NavHierarchyLevelsNum = 8;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxJumpHeight = 1;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxFallHeight = 3;

	UPROPERTY(EditDefaultsOnly)
	int32 NavAgentHeight = 2;

	TArray<TArray<TArray<VoxelEngine::Navigation::NavNode*>>> WalkableVoxelNodes;

	FVoxelNavGenerationFinished GenerationFinishedCallback;

	bool IsVoxelWalkable(const FIntVector& Coord, int32 AgentHeight) const;
	void CreateSiblingLinks(VoxelEngine::Navigation::NavNode*);
};
