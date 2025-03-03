#pragma once

#include "NavNode.h"

namespace VoxelEngine::Navigation
{
	struct FTarjanSccNodePayload
	{
		bool bIsInStack = false;
		int32 Index = INDEX_NONE;
		int32 LowLinks = INDEX_NONE;
	};

	class TarjanScc
	{
	public:
		TarjanScc(TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph);
		~TarjanScc();
		const TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>>& GetSccs();
	private:
		TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph;
		size_t Index = 0;
		TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>> Stack;
		TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>> Components;

		void StrongConnect(TSharedPtr<VoxelEngine::Navigation::NavNode>& Node);
		
	};
}