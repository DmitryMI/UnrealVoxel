#pragma once

#include <cstdint>
#include <memory>
#include "CoreMinimal.h"
#include <functional>

namespace VoxelEngine::DataStructures
{

	template<typename T>
	class TOctreeNode
	{
	public:
		TOctreeNode(const FBox& Boundary) : Boundary(Boundary)
		{
			
		}

		FBox Boundary;
		std::vector<T> Items;
		std::array<std::unique_ptr<TOctreeNode<T>>, 8> ChildOctants;
	};

	template<typename T>
	class TOctree
	{
	public:
		using ItemBoundaryFunc = std::function<FBox(const T&)>;

		TOctree(const FBox& Boundary, ItemBoundaryFunc ItemBoundaryGetter, size_t NodeCapacity = 32, size_t MaxDepth = 128):
			ItemBoundaryGetter(ItemBoundaryGetter),
			NodeCapacity(NodeCapacity),
			MaxDepth(MaxDepth)
		{
			RootNode = std::make_unique<TOctreeNode<T>>(Boundary);
		}

		void Insert(const T& Item)
		{
			Insert(RootNode.get(), Item, 0);
		}

		std::vector<std::shared_ptr<Box>> Search(const FVector& Location)
		{
			std::vector<T> Results;
			Search(RootNode.get(), Location, results);
			return Results;
		}

	private:
		size_t NodeCapacity;
		size_t MaxDepth;
		ItemBoundaryFunc ItemBoundaryGetter;
		
		std::unique_ptr<TOctreeNode<T>> RootNode;

		void Subdivide(TOctreeNode<T>* Node)
		{
			FBox NodeBoundary = Node->Boundary;
			FVector NodeCenter = Node->Boundary.GetCenter();
			// Create 8 children (octants)
			Node->ChildOctants[0] = std::make_unique<TOctreeNode<T>>(
				FBox(
					NodeBoundary.Min, 
					NodeCenter
				)
			);

			Node->ChildOctants[1] = std::make_unique<TOctreeNode<T>>(
				FBox(
					FVector(NodeCenter.X, NodeBoundary.Min.Y, NodeBoundary.Min.Z), 
					FVector(NodeBoundary.Max.X, NodeCenter.Y, NodeCenter.Z)
				)
			);

			Node->ChildOctants[2] = std::make_unique<TOctreeNode<T>>(
				FBox(
					FVector(NodeBoundary.Min.X, NodeCenter.Y, NodeBoundary.Min.Z),
					FVector(NodeCenter.X, NodeBoundary.Max.Y, NodeCenter.Z)
				)

			);

			Node->ChildOctants[3] = std::make_unique<TOctreeNode<T>>(
				FBox(
					FVector(NodeCenter.X, NodeCenter.Y, NodeBoundary.Min.Z),
					FVector(NodeBoundary.Max.X, NodeBoundary.Max.Y, NodeCenter.Z)
				)
			);

			Node->ChildOctants[4] = std::make_unique<TOctreeNode<T>>(
				FBox(
					FVector(NodeBoundary.Min.X, NodeBoundary.Min.Y, NodeCenter.Z),
					FVector(NodeCenter.X, NodeCenter.Y, NodeBoundary.Max.Z)
				)
			);

			Node->ChildOctants[5] = std::make_unique<TOctreeNode<T>>(
				FBox(
					FVector(NodeCenter.X, NodeBoundary.Min.Y, NodeCenter.Z),
					FVector(NodeBoundary.Max.X, NodeCenter.Y, NodeBoundary.Max.Z)
				)
			);

			Node->ChildOctants[6] = std::make_unique<TOctreeNode<T>>(
				FBox(
					FVector(NodeBoundary.Min.X, NodeCenter.Y, NodeCenter.Z),
					FVector(NodeCenter.X, NodeBoundary.Max.Y, NodeBoundary.Max.Z)
				)
			);

			Node->ChildOctants[7] = std::make_unique<TOctreeNode<T>>(
				FBox(
					NodeCenter,
					NodeBoundary.Max
				)
			);
		}
		
		void Insert(TOctreeNode<T>* Node, const T& Item, int Depth)
		{
			if (Depth >= MaxDepth || Node->Items.size() < NodeCapacity)
			{
				Node->Items.push_back(Item);
				return;
			}

			if (!Node->ChildOctants[0])
			{
				Subdivide(Node);
			}

			FBox ItemBoundary = ItemBoundaryGetter(Item);

			for (std::unique_ptr<TOctreeNode<T>>& Child : Node->ChildOctants)
			{
				if (Child->Boundary.Intersects(ItemBoundary))
				{
					Insert(Child.get(), ItemBoundary, Depth + 1);
				}
			}
		}

		void Search(TOctreeNode<T>* Node, const FVector& Location, std::vector<T>& Results) 
		{
			if (!RootNode->Boundary.Contains(Location))
			{
				return;
			}

			for (auto& Item : Node->Items)
			{
				FBox ItemBoundary = ItemBoundaryGetter(Item);
				if (ItemBoundary.Contains(Location))
				{
					Results.push_back(Item);
				}
			}

			for (auto& Child : Node->ChildOctants)
			{
				if (Child)
				{
					Search(Child.get(), Location, Results);
				}
			}
		}
	};
}