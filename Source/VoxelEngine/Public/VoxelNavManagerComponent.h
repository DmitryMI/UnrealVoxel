// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelChange.h"
#include "VoxelEngine/Public/Navigation/NavNode.h"
#include "VoxelEngine/Public/DataStructures/RStarTree.h"
#include "VoxelNavManagerComponent.generated.h"

using NavLevelGrid = TArray<TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>>>;

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

	UPROPERTY(EditAnywhere)
	int DebugDrawNavNodesLevelMin = 0;

	UPROPERTY(EditAnywhere)
	int DebugDrawNavNodesLevelMax = 2;

	UPROPERTY(EditDefaultsOnly)
	int32 NavHierarchyLevelsNum = 3;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxJumpHeight = 1;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxFallHeight = 3;

	UPROPERTY(EditDefaultsOnly)
	int32 NavAgentHeight = 2;

	NavLevelGrid TopLevelNodes;

	FVoxelNavGenerationFinished GenerationFinishedCallback;

	bool IsVoxelWalkable(const FIntVector& Coord, int32 AgentHeight) const;
	void CreateLevelZeroSiblingLinks(VoxelEngine::Navigation::NavNode* WalkableNode, const NavLevelGrid& LevelZeroNodes);

	NavLevelGrid CreateNavLevel(const NavLevelGrid& PreviousLevel, int32 Level);

	void DeepFirstSearch(
		TSharedPtr<VoxelEngine::Navigation::NavNode> Node,
		const TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph,
		TSet<VoxelEngine::Navigation::NavNode*>& VisitedNodes, 
		TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& GraphComponent
	) const;

	TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>> GetGraphComponents(const TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph) const;
	FIntBox GetBoundingBox(const TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph) const;

	void DebugDrawNavNode(VoxelEngine::Navigation::NavNode* Node, int Level) const;
};
