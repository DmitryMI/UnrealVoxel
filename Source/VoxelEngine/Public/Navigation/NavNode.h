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
		NavNode* Parent = nullptr;
		TArray<NavNode*> Children{};

		NavNode(const FIntBox& Bounds, uint8 Level);
		void LinkSibling(NavNode* Sibling, ENavLinkPermissions Permissions);
		void UnlinkSibling(NavNode* Sibling);
		int32 SiblingsNum() const;


		TPair<NavNode*, ENavLinkPermissions> GetSiblingLink(int32 Index) const;

	private:
		TArray<NavNode*> Siblings{};
		TArray<ENavLinkPermissions> SiblingsPermissions{};
	};
}