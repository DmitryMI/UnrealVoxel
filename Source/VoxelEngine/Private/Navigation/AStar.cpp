#include "AStar.h"

namespace VoxelEngine::Navigation
{

	VoxelEngine::Navigation::AStar::AStar(
		const DistanceFunction& Distance,
		const HeuristicFunction& Heuristic,
		const TraversabilityFunction& Traversability,
		const VisitFunction& Visit
	) :
		Distance(Distance),
		Heuristic(Heuristic),
		Traversability(Traversability),
		Visit(Visit)
	{
	}

	AStar::~AStar()
	{
		for (NavNode* Node : NodesWithPayload)
		{
			Node->ClearAlgorithmPayload();
		}
	}

	TUniquePtr<TDoubleLinkedList<NavNode*>> AStar::FindPath(VoxelEngine::Navigation::NavNode* From, VoxelEngine::Navigation::NavNode* To)
	{
		check(Heuristic);
		check(Traversability);

		std::priority_queue<NavNode*, std::vector<NavNode*>, AStarNodePriorityComparator> OpenSet;

		CreatePayload(From);

		GetPayload(From).GScore = 0;
		GetPayload(From).HScore = Heuristic(From, To);

		OpenSet.push(From);
		GetPayload(From).bIsInOpenSet = true;

		while (!OpenSet.empty())
		{
			NavNode* Current = OpenSet.top();
			if (Visit)
			{
				Visit(Current, true);
			}

			OpenSet.pop();
			if (Current == To)
			{
				return ReconstructPath(To);
			}

			GetPayload(Current).bIsInOpenSet = false;

			for (int I = 0; I < Current->SiblingsNum(); I++)
			{
				if (!Traversability(Current, I))
				{
					continue;
				}

				NavNode* Sibling = Current->GetSiblingLink(I).Key.Pin().Get();

				if (Visit)
				{
					Visit(Sibling, false); 
				}
				
				double DistanceToSibling = Distance(Current, Sibling);
				double SiblingHeuristic = Heuristic(Sibling, To);

				double TentativeGCost = GetPayload(Current).GScore + DistanceToSibling;
				CreatePayload(Sibling);
				
				if (TentativeGCost < GetPayload(Sibling).GScore)
				{
					GetPayload(Sibling).Predecessor = Current;
					GetPayload(Sibling).GScore = TentativeGCost;
					GetPayload(Sibling).HScore = SiblingHeuristic;
					if (!GetPayload(Sibling).bIsInOpenSet)
					{
						OpenSet.push(Sibling);
						GetPayload(From).bIsInOpenSet = true;
					}
				}
			}
		}

		return nullptr;
	}

	void AStar::CreatePayload(VoxelEngine::Navigation::NavNode* Node)
	{
		if (Node->HasAlgorithmPayload<FAStarNodePayload>())
		{
			return;
		}
		Node->SetAlgorithmPayload<FAStarNodePayload>(FAStarNodePayload());
		NodesWithPayload.Add(Node);
	}

	FAStarNodePayload& AStar::GetPayload(NavNode* Node)
	{
		return Node->GetAlgorithmPayload<FAStarNodePayload>();
	}

	TUniquePtr<TDoubleLinkedList<NavNode*>> AStar::ReconstructPath(VoxelEngine::Navigation::NavNode* Node) const
	{
		TUniquePtr<TDoubleLinkedList<NavNode*>> PathList{new TDoubleLinkedList<NavNode*>()};
		while (Node)
		{
			PathList->AddHead(Node);
			Node = GetPayload(Node).Predecessor;
		}
		return PathList;
	}

}