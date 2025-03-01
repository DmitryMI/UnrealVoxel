#pragma once

#include "CoreMinimal.h"
#include "VoxelEngine/Public/DataStructures/IntBox.h"
#include "NavLinkPermissions.h"

namespace VoxelEngine::Navigation
{
	class NavNode
	{
	public:
		FIntBox Bounds;
		uint8 Level;
		TWeakPtr<NavNode> Parent = nullptr;
		TArray<TSharedPtr<NavNode>> Children{};

		NavNode(const FIntBox& Bounds, uint8 Level);
		void LinkSibling(TWeakPtr<NavNode> Sibling, TArray<ENavLinkPermissions> Permissions);
		void UnlinkSibling(NavNode* Sibling);
		int32 SiblingsNum() const;

		TPair<TWeakPtr<NavNode>, TArray<ENavLinkPermissions>> GetSiblingLink(int32 Index) const;

	private:
		TArray<TWeakPtr<NavNode>> Siblings{};
		TArray<TArray<ENavLinkPermissions>> SiblingsPermissions{};
	};
}