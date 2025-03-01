// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelNavManagerComponent.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"
#include "VoxelQueryUtils.h"

// Sets default values for this component's properties
UVoxelNavManagerComponent::UVoxelNavManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UVoxelNavManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

bool UVoxelNavManagerComponent::IsVoxelWalkable(const FIntVector& Coord, int32 AgentHeight) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	check(AgentHeight > 0);
	
	if (VoxelWorld->IsVoxelTraversable(Coord))
	{
		return false;
	}

	for (int Z = 1; Z < AgentHeight; Z++)
	{
		if (!VoxelWorld->IsVoxelTraversable(Coord + FIntVector(0, 0, Z)))
		{
			return false;
		}
	}
	
	return true;
}

void UVoxelNavManagerComponent::CreateSiblingLinks(VoxelEngine::Navigation::NavNode* Node)
{
	const FIntVector& Coord = Node->Bounds.Min;

	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	int32 WorldHeight = VoxelWorld->GetWorldHeight();

	const TStaticArray<FIntVector, 4> SiblingsOffsetsXy{
		FIntVector(1, 0, 0),
		FIntVector(0, 1, 0),
		FIntVector(-1, 0, 0),
		FIntVector(0, -1, 0),
	};

	int32 MinHeight = FMath::Max(0, Coord.Z - MaxFallHeight);
	int32 MaxHeight = FMath::Min(WorldHeight, Coord.Z + MaxJumpHeight);
	
	for (const FIntVector& Offset : SiblingsOffsetsXy)
	{
		FIntVector ColumnCoord = Coord + Offset;
		if (!VoxelWorld->IsValidCoordinate(ColumnCoord))
		{
			continue;
		}
		const TArray<VoxelEngine::Navigation::NavNode*>& NodesColumn = WalkableVoxelNodes[ColumnCoord.X][ColumnCoord.Y];
		int LastSolidZ = WorldHeight + 1;
		for (auto* OffsetNode : NodesColumn)
		{
			int32 Z = OffsetNode->Bounds.Min.Z;
			LastSolidZ = Z;
			if (MinHeight > Z || Z > MaxHeight)
			{
				continue;
			}

			if (Z == Coord.Z)
			{
				Node->LinkSibling(OffsetNode, VoxelEngine::Navigation::ENavLinkPermissions::None);
				continue;
			}

			if (Z > Coord.Z)
			{
				Node->LinkSibling(OffsetNode, VoxelEngine::Navigation::ENavLinkPermissions::JumpUp);
				continue;
			}

			if (LastSolidZ - Z > NavAgentHeight)
			{
				continue;
			}

			Node->LinkSibling(OffsetNode, VoxelEngine::Navigation::ENavLinkPermissions::JumpDown);
		}
	}
}

void UVoxelNavManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	if (bDebugDrawNavVolumes)
	{
		for (int X = 0; X < WalkableVoxelNodes.Num(); X++)
		{
			for (int Y = 0; Y < WalkableVoxelNodes[X].Num(); Y++)
			{
				for (auto* NavNode : WalkableVoxelNodes[X][Y])
				{
					double VoxelSize = VoxelWorld->GetVoxelSizeWorld();
					FBox NodeBox = NavNode->Bounds.ToBox(VoxelSize).ShiftBy(FVector::UpVector * VoxelSize);
					FIntVector NodeCoord = NavNode->Bounds.Min;
					// NodeBox = NodeBox.ExpandBy(-FVector(VoxelSize, VoxelSize, VoxelSize) / 10);
					// DrawDebugBox(GetWorld(), NodeBox.GetCenter(), NodeBox.GetExtent(), FColor::Magenta);
					for (int32 I = 0; I < NavNode->SiblingsNum(); I++)
					{
						auto SiblingPermissionPair = NavNode->GetSiblingLink(I);
						auto* Sibling = SiblingPermissionPair.Key;
						FBox SiblingBox = Sibling->Bounds.ToBox(VoxelSize).ShiftBy(FVector::UpVector * VoxelSize);
						
						FIntVector SiblingCoord = Sibling->Bounds.Min;
						FIntVector SiblingOffset = SiblingCoord - NodeCoord;
						FVector ArrowOffset;
						if (SiblingOffset.X > 0 && SiblingOffset.Y == 0)
						{
							ArrowOffset = FVector::RightVector * VoxelSize * 0.1;
						}
						else if (SiblingOffset.X == 0 && SiblingOffset.Y > 0)
						{
							ArrowOffset = -FVector::ForwardVector * VoxelSize * 0.1;
						}
						else if (SiblingOffset.X < 0 && SiblingOffset.Y == 0)
						{
							ArrowOffset = -FVector::RightVector * VoxelSize * 0.1;
						}
						else if (SiblingOffset.X == 0 && SiblingOffset.Y < 0)
						{
							ArrowOffset = FVector::ForwardVector * VoxelSize * 0.1;
						}
						
						auto Permissions = SiblingPermissionPair.Value;
						FColor LineColor = FColor::Blue;
						if (VoxelEngine::Navigation::HasFlags(Permissions, VoxelEngine::Navigation::ENavLinkPermissions::JumpUp))
						{
							LineColor = FColor::Green;
						}
						else if (VoxelEngine::Navigation::HasFlags(Permissions, VoxelEngine::Navigation::ENavLinkPermissions::JumpDown))
						{
							LineColor = FColor::Red;
						}
						DrawDebugDirectionalArrow(GetWorld(), NodeBox.GetCenter() + ArrowOffset, SiblingBox.GetCenter() + ArrowOffset, VoxelSize / 4, LineColor);
					}
				}
			}
		}
	}
}

void UVoxelNavManagerComponent::GenerateNavData(const FVoxelNavGenerationFinished& Callback)
{
	WalkableVoxelNodes.Empty();

	GenerationFinishedCallback = Callback;
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	FIntVector WorldSizeVoxel = VoxelWorld->GetWorldSizeVoxel();

	WalkableVoxelNodes.SetNum(WorldSizeVoxel.X);
	for (int X = 0; X < WorldSizeVoxel.X; X++)
	{
		WalkableVoxelNodes[X].SetNum(WorldSizeVoxel.Y);
	}
	
	for (int X = 0; X < WorldSizeVoxel.X; X++)
	{
		for (int Y = 0; Y < WorldSizeVoxel.Y; Y++)
		{
			for (int Z = WorldSizeVoxel.Z - 1; Z >= 0; Z--)
			{
				FIntVector VoxelCoord{ X, Y, Z };
				if (!IsVoxelWalkable(VoxelCoord, NavAgentHeight))
				{
					continue;
				}

				auto* Node = new VoxelEngine::Navigation::NavNode(FIntBox(VoxelCoord, VoxelCoord), 0);
				WalkableVoxelNodes[X][Y].Add(Node);
			}
		}
	}

	for (int X = 0; X < WorldSizeVoxel.X; X++)
	{
		for (int Y = 0; Y < WorldSizeVoxel.Y; Y++)
		{
			for (auto* NavNode : WalkableVoxelNodes[X][Y])
			{
				CreateSiblingLinks(NavNode);
			}
		}
	}

	GenerationFinishedCallback.ExecuteIfBound();
}

void UVoxelNavManagerComponent::ChangeVoxel(const FVoxelChange& VoxelChange)
{
}

