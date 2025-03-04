// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelChange.h"
#include "VoxelEngine/Public/Navigation/NavNode.h"
#include "VoxelEngine/Public/DataStructures/RStarTree.h"
#include "VoxelNavLinkPermissions.h"
#include "VoxelNavManagerComponent.generated.h"

using NavLevelGrid = TArray<TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>>>;

USTRUCT(BlueprintType)
struct FVoxelNavQueryParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EVoxelNavLinkPermissions NavLinkPermissions;
};

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

	UFUNCTION(BlueprintCallable)
	void DebugDrawNavHierarchy(const FIntVector& Voxel);

	UFUNCTION(BlueprintCallable)
	bool CheckPathExists(const FVector& From, const FVector& To, const FVoxelNavQueryParams& Params) const;

	UFUNCTION(BlueprintCallable)
	TArray<FBox> DebugFindPath(const FVector& From, const FVector& To, TArray<FBox>& OutVisitedNodes, TArray<bool>& OutVisitedNodesFlags);

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

	NavLevelGrid WalkableVoxelNodes;
	NavLevelGrid TopLevelNodes;

	FVoxelNavGenerationFinished GenerationFinishedCallback;

	bool IsVoxelWalkable(const FIntVector& Coord, int32 AgentHeight) const;
	void CreateLevelZeroSiblingLinks(VoxelEngine::Navigation::NavNode* WalkableNode, const NavLevelGrid& LevelZeroNodes);

	void CreateNavLevelNodes(NavLevelGrid& LevelGrid, const NavLevelGrid& PreviousLevel, int32 X, int32 Y, int32 Level) const;
	void LinkNavLevelNodes(NavLevelGrid& LevelGrid, int32 X, int32 Y) const;
	void LinkNavLevelNodes(VoxelEngine::Navigation::NavNode* Node) const;
	NavLevelGrid CreateNavLevel(const NavLevelGrid& PreviousLevel, int32 Level);

	void DeepFirstSearch(
		TSharedPtr<VoxelEngine::Navigation::NavNode> Node,
		const TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph,
		TSet<VoxelEngine::Navigation::NavNode*>& VisitedNodes, 
		TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& GraphComponent
	) const;

	TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>> GetGraphComponents(TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph) const;
	FIntBox GetBoundingBox(const TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph) const;

	void DebugDrawNavNode(VoxelEngine::Navigation::NavNode* Node, int RecursionDirection) const;

	TWeakPtr<VoxelEngine::Navigation::NavNode> ProjectOntoWalkableNode(const FVector& Location) const;
	TWeakPtr<VoxelEngine::Navigation::NavNode> ProjectOntoWalkableNode(const FIntVector& Coord) const;
	bool CheckPathExists(VoxelEngine::Navigation::NavNode* From, VoxelEngine::Navigation::NavNode* To, const FVoxelNavQueryParams& Params) const;

	/// <summary>
	/// <para>Tries to find common parent with the lowest level.</para>
	/// <para>OutParentA and OutParentB will point to top level parents if common parent could not be found.</para>
	/// <para>OutParentA and OutParentB will point to the same common parent if it was found.</para>
	/// </summary>
	/// <param name="NodeA">Starting node A</param>
	/// <param name="NodeB">Starting node B</param>
	/// <param name="OutParentA">Closest parent of A</param>
	/// <param name="OutParentB">Closest parent of B</param>
	/// <returns>True if OutParentA and OutParentB point to the same node (common parent was found)</returns>
	bool GetClosestCommonParent(VoxelEngine::Navigation::NavNode* NodeA, VoxelEngine::Navigation::NavNode* NodeB, VoxelEngine::Navigation::NavNode*& OutParentA, VoxelEngine::Navigation::NavNode*& OutParentB) const;
};
