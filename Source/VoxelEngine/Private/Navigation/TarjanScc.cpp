#pragma once

#include "TarjanScc.h"
#include "NavNode.h"

namespace VoxelEngine::Navigation
{
	VoxelEngine::Navigation::TarjanScc::TarjanScc(TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>& Graph):
		Graph(Graph)
	{
		for (auto& Node : Graph)
		{
			Node->AlgorithmPayload = FTarjanSccNodePayload();
		}
	}

	TarjanScc::~TarjanScc()
	{
		for (auto& Node : Graph)
		{
			Node->AlgorithmPayload = nullptr;
		}
	}

	void TarjanScc::StrongConnect(TSharedPtr<VoxelEngine::Navigation::NavNode>& Node)
	{
		if (!Node->HasAlgorithmPayload<FTarjanSccNodePayload>())
		{
			return;
		}

		FTarjanSccNodePayload& NodePayload = Node->GetAlgorithmPayload<FTarjanSccNodePayload>();

		NodePayload.Index = Index;
		NodePayload.LowLinks = Index;
		Index++;
		Stack.Add(Node);
		NodePayload.bIsInStack = true;

		for (int I = 0; I < Node->SiblingsNum(); I++)
		{
			TWeakPtr<VoxelEngine::Navigation::NavNode> SiblingNodeWeak = Node->GetSiblingLink(I).Key;
			TSharedPtr<VoxelEngine::Navigation::NavNode> SiblingNode = SiblingNodeWeak.Pin();
			if (!SiblingNode->HasAlgorithmPayload<FTarjanSccNodePayload>())
			{
				continue;
			}
			FTarjanSccNodePayload& SiblingNodePayload = SiblingNode->GetAlgorithmPayload<FTarjanSccNodePayload>();

			if (SiblingNodePayload.Index == INDEX_NONE)
			{
				StrongConnect(SiblingNode);
				int32& NodeLowLinks = NodePayload.LowLinks;
				int32& SiblingLowLinks = SiblingNodePayload.LowLinks;
				NodeLowLinks = FMath::Min(NodeLowLinks, SiblingLowLinks);
			}
			else if (SiblingNodePayload.bIsInStack)
			{
				int32& NodeLowLinks = NodePayload.LowLinks;
				int32& SiblingIndex = SiblingNodePayload.Index;
				NodeLowLinks = FMath::Min(NodeLowLinks, SiblingIndex);
			}
		}

		if (NodePayload.LowLinks == NodePayload.Index)
		{
			TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>> Scc{};
			while (true)
			{
				const auto W = Stack.Pop();
				W->GetAlgorithmPayload<FTarjanSccNodePayload>().bIsInStack = false;
				Scc.Add(W);
				if (W.Get() == Node.Get())
				{
					break;
				}
			}
			Components.Add(Scc);
		}
	}

	const TArray<TArray<TSharedPtr<VoxelEngine::Navigation::NavNode>>>& TarjanScc::GetSccs()
	{
		for (auto& Node : Graph)
		{
			if (Node->GetAlgorithmPayload<FTarjanSccNodePayload>().Index == INDEX_NONE)
			{
				StrongConnect(Node);
			}
		}
		return Components;
	}
}