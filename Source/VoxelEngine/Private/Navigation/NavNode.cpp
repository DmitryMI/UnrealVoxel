#include "VoxelEngine/Public/Navigation/NavNode.h"

namespace VoxelEngine::Navigation
{

	VoxelEngine::Navigation::NavNode::NavNode(const FIntBox& Bounds, uint8 Level) :
		Bounds(Bounds),
		Level(Level)
	{
	}

	void VoxelEngine::Navigation::NavNode::LinkSibling(NavNode* Sibling, ENavLinkPermissions Permissions)
	{
		Siblings.Add(Sibling);
		SiblingsPermissions.Add(Permissions);
	}

	void VoxelEngine::Navigation::NavNode::UnlinkSibling(NavNode* Sibling)
	{
		int SiblingIndex = Siblings.IndexOfByKey(Sibling);
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

	TPair<NavNode*, ENavLinkPermissions> NavNode::GetSiblingLink(int32 Index) const
	{
		return { Siblings[Index], SiblingsPermissions[Index] };
	}
}