// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelNavManagerComponent.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"
#include "VoxelQueryUtils.h"
#include "VoxelEngine/Public/Navigation/TarjanScc.h"
#include "VoxelEngine/Public/Navigation/AStar.h"
#include "VoxelEngine/Public/VoxelNavLinkPermissions.h"

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

void UVoxelNavManagerComponent::CreateLevelZeroSiblingLinks(VoxelEngine::Navigation::NavNode* Node, const NavLevelGrid& LevelZeroNodes)
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
		const auto& NodesColumn = LevelZeroNodes[ColumnCoord.X][ColumnCoord.Y];
		int LastSolidZ = WorldHeight + 1;
		for (const auto& OffsetNode : NodesColumn)
		{
			int32 Z = OffsetNode->Bounds.Min.Z;

			if (MinHeight > Z || Z > MaxHeight)
			{
				LastSolidZ = Z;
				continue;
			}

			if (Z == Coord.Z)
			{
				Node->LinkSibling(OffsetNode, { EVoxelNavLinkPermissions::None });
				LastSolidZ = Z;
				continue;
			}

			if (Z > Coord.Z)
			{
				Node->LinkSibling(OffsetNode, { EVoxelNavLinkPermissions::JumpUp });
				LastSolidZ = Z;
				continue;
			}

			if (LastSolidZ - Node->Bounds.Min.Z < NavAgentHeight)
			{
				LastSolidZ = Z;
				continue;
			}

			Node->LinkSibling(OffsetNode, { EVoxelNavLinkPermissions::JumpDown });
			LastSolidZ = Z;
		}
	}
}

void UVoxelNavManagerComponent::CreateNavLevelNodes(NavLevelGrid& LevelGrid, const NavLevelGrid& PreviousLevel, int32 X, int32 Y, int32 Level) const
{
	int PrevLevelMinX = X * 2;
	int PrevLevelMaxX = (X + 1) * 2;
	int PrevLevelMinY = Y * 2;
	int PrevLevelMaxY = (Y + 1) * 2;

	TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>> CellNodes;
	for (int PrevX = PrevLevelMinX; PrevX < PrevLevelMaxX && PrevX < PreviousLevel.Num(); PrevX++)
	{
		for (int PrevY = PrevLevelMinY; PrevY < PrevLevelMaxY && PrevY < PreviousLevel[PrevX].Num(); PrevY++)
		{
			for (const auto& PrevLevelNode : PreviousLevel[PrevX][PrevY])
			{
				CellNodes.Add(PrevLevelNode);
			}
		}
	}

	auto GraphComponents = GetGraphComponents(CellNodes);
	for (const auto& Component : GraphComponents)
	{
		FIntBox BoundingBox = GetBoundingBox(Component);
		TSharedPtr<VoxelEngine::Navigation::NavNode> Parent = TSharedPtr<VoxelEngine::Navigation::NavNode>(new VoxelEngine::Navigation::NavNode(BoundingBox, Level));
		for (const auto& Node : Component)
		{
			Parent->Children.Add(Node);
			Node->Parent = Parent;
		}
		LevelGrid[X][Y].Add(Parent);
	}
}

void UVoxelNavManagerComponent::LinkNavLevelNodes(NavLevelGrid& LevelGrid, int32 X, int32 Y) const
{
	for (const auto& Node : LevelGrid[X][Y])
	{
		LinkNavLevelNodes(Node.Get());
	}
}

void UVoxelNavManagerComponent::LinkNavLevelNodes(VoxelEngine::Navigation::NavNode* Node) const
{
	check(Node);
	TMap<TSharedPtr<VoxelEngine::Navigation::NavNode>, TArray<EVoxelNavLinkPermissions>> NodeLinkMap;
	for (const auto& Child : Node->Children)
	{
		for (int I = 0; I < Child->SiblingsNum(); I++)
		{
			const auto& ChildSiblingWeak = Child->GetSiblingLink(I).Key;
			auto ChildSiblingLinkRules = Child->GetSiblingLink(I).Value;
			const auto& ChildSibling = ChildSiblingWeak.Pin();
			if (!ChildSibling->Parent.IsValid() || ChildSibling->Parent.Pin().Get() == Node)
			{
				continue;
			}
			const auto& ChildSiblingParent = ChildSibling->Parent.Pin();

			if (NodeLinkMap.Contains(ChildSiblingParent))
			{
				TArray<EVoxelNavLinkPermissions>& LinkRules = NodeLinkMap[ChildSiblingParent];
				for (const auto& ChildSiblingLinkRule : ChildSiblingLinkRules)
				{
					LinkRules.AddUnique(ChildSiblingLinkRule);
				}
			}
			else
			{
				TArray<EVoxelNavLinkPermissions> LinkRules;
				for (const auto& ChildSiblingLinkRule : ChildSiblingLinkRules)
				{
					LinkRules.AddUnique(ChildSiblingLinkRule);
				}
				NodeLinkMap.Add(ChildSiblingParent, LinkRules);
			}
		}
	}

	for (const auto& Link : NodeLinkMap)
	{
		Node->LinkSibling(Link.Key, Link.Value);
	}
}

NavLevelGrid UVoxelNavManagerComponent::CreateNavLevel(const NavLevelGrid& PreviousLevel, int32 Level)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	FIntVector WorldSize = VoxelWorld->GetWorldSizeVoxel();
	int PreviousNodeSize = 1 << (Level - 1);
	int32 NodeSize = 1 << Level;
	int32 GridSizeX = WorldSize.X / NodeSize;
	if (WorldSize.X % NodeSize != 0)
	{
		GridSizeX++;
	}
	int32 GridSizeY = WorldSize.Y / NodeSize;
	if (WorldSize.Y % NodeSize != 0)
	{
		GridSizeY++;
	}

	NavLevelGrid LevelGrid;
	LevelGrid.SetNum(GridSizeX);
	for (int X = 0; X < GridSizeX; X++)
	{
		LevelGrid[X].SetNum(GridSizeY);
	}

	for (int X = 0; X < GridSizeX; X++)
	{
		for (int Y = 0; Y < GridSizeY; Y++)
		{
			CreateNavLevelNodes(LevelGrid, PreviousLevel, X, Y, Level);
		}
	}

	for (int X = 0; X < GridSizeX; X++)
	{
		for (int Y = 0; Y < GridSizeY; Y++)
		{
			LinkNavLevelNodes(LevelGrid, X, Y);
		}
	}

	return LevelGrid;
}

void UVoxelNavManagerComponent::DeepFirstSearch(
	TSharedPtr<VoxelEngine::Navigation::NavNode> Node,
	const TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph,
	TSet<VoxelEngine::Navigation::NavNode*>& VisitedNodes,
	TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& GraphComponent
) const
{
	VisitedNodes.Add(Node.Get());
	GraphComponent.Add(Node);
	for (int I = 0; I < Node->SiblingsNum(); I++)
	{
		const auto& Sibling = Node->GetSiblingLink(I).Key;
		bool bIsVisited = VisitedNodes.Contains(Sibling.Pin().Get());
		if (bIsVisited)
		{
			continue;
		}

		bool bIsInGraph = false;
		for (const auto& GraphNode : Graph)
		{
			if (GraphNode.Get() == Sibling.Pin().Get())
			{
				bIsInGraph = true;
				break;
			}
		}

		if (!bIsInGraph)
		{
			continue;
		}

		DeepFirstSearch(Sibling.Pin(), Graph, VisitedNodes, GraphComponent);
	}
}

TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>> UVoxelNavManagerComponent::GetGraphComponents(TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph) const
{
	VoxelEngine::Navigation::TarjanScc TarjanScc(Graph);
	return TarjanScc.GetSccs();
}

FIntBox UVoxelNavManagerComponent::GetBoundingBox(const TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph) const
{
	if (Graph.Num() == 0)
	{
		return FIntBox();
	}
	FIntVector Min = Graph[0]->Bounds.Min;
	FIntVector Max = Graph[0]->Bounds.Max;
	for (int I = 1; I < Graph.Num(); I++)
	{
		FIntVector NodeMin = Graph[I]->Bounds.Min;
		FIntVector NodeMax = Graph[I]->Bounds.Max;
		for (int Dim = 0; Dim < 3; Dim++)
		{
			if (NodeMin[Dim] < Min[Dim])
			{
				Min[Dim] = NodeMin[Dim];
			}
			if (NodeMax[Dim] > Max[Dim])
			{
				Max[Dim] = NodeMax[Dim];
			}
		}
	}
	return FIntBox(Min, Max);
}

void UVoxelNavManagerComponent::DebugDrawNavNode(VoxelEngine::Navigation::NavNode* Node, int RecursionDirection) const
{
	if (!Node)
	{
		return;
	}

	if (RecursionDirection < 0 && Node->Level > DebugDrawNavNodesLevelMin)
	{
		for (const auto& Child : Node->Children)
		{
			DebugDrawNavNode(Child.Get(), RecursionDirection);
		}
	}
	else if (RecursionDirection > 0 && Node->Level < DebugDrawNavNodesLevelMax)
	{
		if (Node->Parent.IsValid())
		{
			DebugDrawNavNode(Node->Parent.Pin().Get(), RecursionDirection);
		}
	}

	if (DebugDrawNavNodesLevelMin > Node->Level || Node->Level > DebugDrawNavNodesLevelMax)
	{
		return;
	}

	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	double VoxelSize = VoxelWorld->GetVoxelSizeWorld();

	FBox NodeBox = Node->Bounds.ToBox(VoxelSize).ShiftBy(FVector::UpVector * VoxelSize);
	FIntVector NodeCoord = Node->Bounds.Min;

	int32 ColorStep = 255 / NavHierarchyLevelsNum;
	FColor BoxColor = FColor::Black;
	BoxColor.R = Node->Level * ColorStep;
	BoxColor.G = 255 - Node->Level * ColorStep;
	// BoxColor.R = 100;

	DrawDebugBox(GetWorld(), NodeBox.GetCenter(), NodeBox.GetExtent(), BoxColor);

	for (int I = 0; I < Node->SiblingsNum(); I++)
	{
		const auto& Sibling = Node->GetSiblingLink(I).Key;
		auto LinkRules = Node->GetSiblingLink(I).Value;

		FBox SiblingBox = Sibling.Pin()->Bounds.ToBox(VoxelSize).ShiftBy(FVector::UpVector * VoxelSize);

		FIntVector SiblingCoord = Sibling.Pin()->Bounds.Min;
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

		check(LinkRules.Num() > 0);
		int LinkIndex = 0;
		for (const auto& LinkRule : LinkRules)
		{
			FVector ArrowVerticalOffset = FVector{ 0, 0, VoxelSize * 0.1 * LinkIndex };
			FColor LineColor = FColor::Blue;
			if (NavLinkPermissionsHasAllFlags(LinkRule, EVoxelNavLinkPermissions::JumpUp))
			{
				LineColor = FColor::Green;
			}
			else if (NavLinkPermissionsHasAllFlags(LinkRule, EVoxelNavLinkPermissions::JumpDown))
			{
				LineColor = FColor::Red;
			}
			DrawDebugDirectionalArrow(GetWorld(), NodeBox.GetCenter() + ArrowOffset + ArrowVerticalOffset, SiblingBox.GetCenter() + ArrowOffset + ArrowVerticalOffset, VoxelSize / 4, LineColor);
			LinkIndex++;
		}
	}
}

TWeakPtr<VoxelEngine::Navigation::NavNode> UVoxelNavManagerComponent::ProjectOntoWalkableNode(const FVector& Location) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	FIntVector Coord = VoxelWorld->GetVoxelCoordFromWorld(Location);
	return ProjectOntoWalkableNode(Coord);
}

TWeakPtr<VoxelEngine::Navigation::NavNode> UVoxelNavManagerComponent::ProjectOntoWalkableNode(const FIntVector& Coord) const
{
	if (WalkableVoxelNodes.Num() == 0)
	{
		return nullptr;
	}

	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	if (!VoxelWorld->IsValidCoordinate(Coord))
	{
		return nullptr;
	}

	check(WalkableVoxelNodes.Num() > Coord.X);
	check(WalkableVoxelNodes[Coord.X].Num() > Coord.Y);

	const auto& NodeColumn = WalkableVoxelNodes[Coord.X][Coord.Y];
	for (const auto& Node : NodeColumn)
	{
		if (Node->Bounds.Min.X == Coord.X && Node->Bounds.Min.Y == Coord.Y && Node->Bounds.Min.Z <= Coord.Z)
		{
			return Node;
		}
	}

	return nullptr;
}

bool UVoxelNavManagerComponent::CheckPathExists(VoxelEngine::Navigation::NavNode* From, VoxelEngine::Navigation::NavNode* To, const FVoxelNavQueryParams& Params) const
{
	VoxelEngine::Navigation::NavNode* FromParent;
	VoxelEngine::Navigation::NavNode* ToParent;
	if (GetClosestCommonParent(From, To, FromParent, ToParent))
	{
		return true;
	}

	return false;
}

bool UVoxelNavManagerComponent::GetClosestCommonParent(VoxelEngine::Navigation::NavNode* NodeA, VoxelEngine::Navigation::NavNode* NodeB, VoxelEngine::Navigation::NavNode*& OutParentA, VoxelEngine::Navigation::NavNode*& OutParentB) const
{
	OutParentA = nullptr;
	OutParentB = nullptr;
	if (!NodeA || !NodeB)
	{
		return false;
	}

	while (NodeA->Parent.IsValid() && NodeB->Parent.IsValid() && NodeA != NodeB)
	{
		NodeA = NodeA->Parent.Pin().Get();
		NodeB = NodeB->Parent.Pin().Get();
	}

	check(NodeA);
	check(NodeB);
	check(NodeA->Parent.IsValid() == NodeB->Parent.IsValid());

	if (NodeA == NodeB)
	{
		OutParentA = NodeA;
		OutParentB = NodeB;
		return true;
	}

	return false;
}

bool UVoxelNavManagerComponent::CheckPathExists(const FVector& From, const FVector& To, const FVoxelNavQueryParams& Params) const
{
	const auto& NodeFrom = ProjectOntoWalkableNode(From);
	if (!NodeFrom.IsValid())
	{
		return false;
	}
	const auto& NodeTo = ProjectOntoWalkableNode(To);
	if (!NodeTo.IsValid())
	{
		return false;
	}

	return CheckPathExists(NodeFrom.Pin().Get(), NodeTo.Pin().Get(), Params);
}

TArray<FBox> UVoxelNavManagerComponent::DebugFindPath(const FVector& From, const FVector& To, TArray<FBox>& OutVisitedNodes, TArray<bool>& OutVisitedNodesFlags)
{
	const auto& NodeFrom = ProjectOntoWalkableNode(From);
	if (!NodeFrom.IsValid())
	{
		return TArray<FBox>();
	}
	const auto& NodeTo = ProjectOntoWalkableNode(To);
	if (!NodeTo.IsValid())
	{
		return TArray<FBox>();
	}

	FVoxelNavQueryParams Params;
	Params.NavLinkPermissions = EVoxelNavLinkPermissions::None | EVoxelNavLinkPermissions::JumpUp | EVoxelNavLinkPermissions::JumpDown;

	double VoxelSize = GetOwner<AVoxelWorld>()->GetVoxelSizeWorld();
	auto Distance = [VoxelSize](VoxelEngine::Navigation::NavNode* From, VoxelEngine::Navigation::NavNode* To) {
		FVector FromCenter = From->Bounds.ToBox(VoxelSize).GetCenter();
		FVector ToCenter = To->Bounds.ToBox(VoxelSize).GetCenter();
		FVector Delta = (ToCenter - FromCenter).GetAbs();
		double DeltaMahnatten = Delta.X + Delta.Y + Delta.Z;
		return DeltaMahnatten;
		};
	auto Heuristic = [VoxelSize](VoxelEngine::Navigation::NavNode* From, VoxelEngine::Navigation::NavNode* To) {
		return (From->Bounds.ToBox(VoxelSize).GetCenter() - To->Bounds.ToBox(VoxelSize).GetCenter()).Size();
		};
	auto Traversability = [Params](VoxelEngine::Navigation::NavNode* From, int LinkIndex) {
		auto LinkRules = From->GetSiblingLink(LinkIndex).Value;
		for (const auto& LinkRule : LinkRules)
		{
			if (LinkRule == EVoxelNavLinkPermissions::None || NavLinkPermissionsHasAnyFlag(LinkRule, Params.NavLinkPermissions))
			{
				return true;
			}
		}
		return false;
		};

	auto Visit = [&](VoxelEngine::Navigation::NavNode* Node, bool bIsFromOpenSet) {
		FBox NodeBox = Node->Bounds.ToBox(VoxelSize).ShiftBy(FVector::UpVector * VoxelSize);
		OutVisitedNodes.Add(NodeBox);
		OutVisitedNodesFlags.Add(bIsFromOpenSet);
		};
	VoxelEngine::Navigation::AStar AStar(Distance, Heuristic, Traversability, Visit);

	TArray<FBox> PathResult;
	auto PathList = AStar.FindPath(NodeFrom.Pin().Get(), NodeTo.Pin().Get());
	if (!PathList)
	{
		return PathResult;
	}
	
	PathResult.Reserve(PathList->Num());

	for (TDoubleLinkedList<VoxelEngine::Navigation::NavNode*>::TIterator It(PathList->GetHead()); It; ++It)
	{
		PathResult.Add(It->Bounds.ToBox(VoxelSize).ShiftBy(FVector::UpVector * VoxelSize));
	}

	return PathResult;
}

void UVoxelNavManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	if (bDebugDrawNavVolumes)
	{
		for (int X = 0; X < TopLevelNodes.Num(); X++)
		{
			for (int Y = 0; Y < TopLevelNodes[X].Num(); Y++)
			{
				for (const auto& Node : TopLevelNodes[X][Y])
				{
					DebugDrawNavNode(Node.Get(), -1);
				}
			}
		}
	}
}

void UVoxelNavManagerComponent::GenerateNavData(const FVoxelNavGenerationFinished& Callback)
{
	TopLevelNodes.Empty();
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

				auto Node = TSharedPtr<VoxelEngine::Navigation::NavNode>(new VoxelEngine::Navigation::NavNode(FIntBox(VoxelCoord, VoxelCoord), 0));
				WalkableVoxelNodes[X][Y].Add(Node);
			}
		}
	}

	for (int X = 0; X < WorldSizeVoxel.X; X++)
	{
		for (int Y = 0; Y < WorldSizeVoxel.Y; Y++)
		{
			for (const auto& NavNode : WalkableVoxelNodes[X][Y])
			{
				CreateLevelZeroSiblingLinks(NavNode.Get(), WalkableVoxelNodes);
			}
		}
	}

	TopLevelNodes = WalkableVoxelNodes;

	for (int32 Level = 1; Level < NavHierarchyLevelsNum; Level++)
	{
		TopLevelNodes = CreateNavLevel(TopLevelNodes, Level);
	}

	GenerationFinishedCallback.ExecuteIfBound();
}

void UVoxelNavManagerComponent::ChangeVoxel(const FVoxelChange& VoxelChange)
{
}

void UVoxelNavManagerComponent::DebugDrawNavHierarchy(const FIntVector& Voxel)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	if (!VoxelWorld->IsValidCoordinate(Voxel))
	{
		return;
	}

	check(WalkableVoxelNodes.Num() > Voxel.X);
	check(WalkableVoxelNodes[Voxel.X].Num() > Voxel.Y);

	const auto& NodeColumn = WalkableVoxelNodes[Voxel.X][Voxel.Y];

	for (const auto& Node : NodeColumn)
	{
		if (Node->Bounds.Min == Voxel)
		{
			DebugDrawNavNode(Node.Get(), 1);
			// break;
		}
	}
}
