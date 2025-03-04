#pragma once

#include "VoxelEngine/Public/Navigation/NavNode.h"
#include <queue>
#include "Containers/List.h"

namespace VoxelEngine::Navigation
{
	using TraversabilityFunction = std::function<bool(VoxelEngine::Navigation::NavNode* From, int)>;
	using HeuristicFunction = std::function<double(VoxelEngine::Navigation::NavNode* From, VoxelEngine::Navigation::NavNode* To)>;
	using VisitFunction = std::function<void(VoxelEngine::Navigation::NavNode* Node, bool)>;

	struct FAStarNodePayload
	{
		bool bIsInOpenSet = false;

		/// <summary>
		/// Summary cost to reach current node from start
		/// </summary>
		double GScore = std::numeric_limits<double>::infinity();

		/// <summary>
		/// Estimated cost to reach end from current node
		/// </summary>
		double HScore = std::numeric_limits<double>::infinity();

		/// <summary>
		/// Node from which we came to current node
		/// </summary>
		VoxelEngine::Navigation::NavNode* Predecessor = nullptr;

		/// <summary>
		/// Total estimated cost of path through current node
		/// </summary>
		/// <returns></returns>
		double FScore() const
		{
			return GScore + HScore;
		}
	};

	
	class AStar
	{
	public:
		AStar(
			const HeuristicFunction& Heuristic,
			const TraversabilityFunction& Traversability,
			const VisitFunction& Visit = nullptr
		);
		~AStar();

		TUniquePtr<TDoubleLinkedList<NavNode*>> FindPath(VoxelEngine::Navigation::NavNode* From, VoxelEngine::Navigation::NavNode* To);

	private:
		class AStarNodePriorityComparator
		{
		public:
			bool operator()(VoxelEngine::Navigation::NavNode* Left, VoxelEngine::Navigation::NavNode* Right) const
			{
				return GetPayload(Left).FScore() > GetPayload(Right).FScore();
			}
		};


		HeuristicFunction Heuristic;
		TraversabilityFunction Traversability;
		VisitFunction Visit;

		TArray<NavNode*> NodesWithPayload;

		void CreatePayload(VoxelEngine::Navigation::NavNode* Node);
		static FAStarNodePayload& GetPayload(VoxelEngine::Navigation::NavNode* Node);
		TUniquePtr<TDoubleLinkedList<NavNode*>> ReconstructPath(VoxelEngine::Navigation::NavNode* Node) const;
	};
}