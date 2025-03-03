#include "VoxelEngine/Public/Navigation/NavNode.h"

namespace VoxelEngine::Navigation
{

	VoxelEngine::Navigation::NavNode::NavNode(const FIntBox& Bounds, uint8 Level) :
		Bounds(Bounds),
		Level(Level)
	{
	}

	void VoxelEngine::Navigation::NavNode::LinkSibling(TWeakPtr<NavNode> Sibling, TArray<EVoxelNavLinkPermissions> Permissions)
	{
		Siblings.Add(Sibling);
		SiblingsPermissions.Add(Permissions);
	}

	void VoxelEngine::Navigation::NavNode::UnlinkSibling(NavNode* Sibling)
	{
		int SiblingIndex = INDEX_NONE;
		for (int I = 0; I < Siblings.Num(); I++)
		{
			if (Siblings[I].Pin().Get() == Sibling)
			{
				SiblingIndex = I;
				break;
			}
		}
		if (SiblingIndex == INDEX_NONE)
		{
			return;
		}
		Siblings.RemoveAt(SiblingIndex);
		SiblingsPermissions.RemoveAt(SiblingIndex);
	}

	int32 VoxelEngine::Navigation::NavNode::SiblingsNum() const
	{
		return Siblings.Num();
	}

	TPair<TWeakPtr<NavNode>, TArray<EVoxelNavLinkPermissions>> NavNode::GetSiblingLink(int32 Index) const
	{
		return { Siblings[Index], SiblingsPermissions[Index] };
	}
}