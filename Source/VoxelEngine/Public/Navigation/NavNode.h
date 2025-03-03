#pragma once

#include "CoreMinimal.h"
#include "VoxelEngine/Public/DataStructures/IntBox.h"
#include "NavLinkPermissions.h"
#include <any>

namespace VoxelEngine::Navigation
{
	class NavNode
	{
	public:
		FIntBox Bounds;
		uint8 Level;
		TWeakPtr<NavNode> Parent = nullptr;
		TArray<TSharedPtr<NavNode>> Children{};
		std::any AlgorithmPayload = nullptr;

		NavNode(const FIntBox& Bounds, uint8 Level);
		void LinkSibling(TWeakPtr<NavNode> Sibling, TArray<ENavLinkPermissions> Permissions);
		void UnlinkSibling(NavNode* Sibling);
		int32 SiblingsNum() const;

		TPair<TWeakPtr<NavNode>, TArray<ENavLinkPermissions>> GetSiblingLink(int32 Index) const;

		template<typename T>
		const T& GetAlgorithmPayload() const
		{
			return std::any_cast<const T&>(AlgorithmPayload);
		}

		template<typename T>
		T& GetAlgorithmPayload()
		{
			return std::any_cast<T&>(AlgorithmPayload);
		}

		template<typename T>
		bool HasAlgorithmPayload() const
		{
			return AlgorithmPayload.type() == typeid(T);
		}

	private:
		TArray<TWeakPtr<NavNode>> Siblings{};
		TArray<TArray<ENavLinkPermissions>> SiblingsPermissions{};
	};
}