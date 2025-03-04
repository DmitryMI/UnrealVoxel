#pragma once

#include "CoreMinimal.h"
#include "VoxelEngine/Public/DataStructures/IntBox.h"
#include "VoxelEngine/Public/VoxelNavLinkPermissions.h"
#include <any>
#include <map>
#include <thread>

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
		void LinkSibling(TWeakPtr<NavNode> Sibling, TArray<EVoxelNavLinkPermissions> Permissions);
		void UnlinkSibling(NavNode* Sibling);
		int32 SiblingsNum() const;

		TPair<TWeakPtr<NavNode>, TArray<EVoxelNavLinkPermissions>> GetSiblingLink(int32 Index) const;

		void ClearAlgorithmPayload()
		{
			AlgorithmPayload.erase(std::this_thread::get_id());
		}

		template<typename T>
		void SetAlgorithmPayload(const T& Value)
		{
			AlgorithmPayload[std::this_thread::get_id()] = Value;
		}

		template<typename T>
		const T& GetAlgorithmPayload() const
		{
			return std::any_cast<const T&>(AlgorithmPayload.at(std::this_thread::get_id()));
		}

		template<typename T>
		T& GetAlgorithmPayload()
		{
			return std::any_cast<T&>(AlgorithmPayload.at(std::this_thread::get_id()));
		}

		template<typename T>
		bool HasAlgorithmPayload() const
		{
			if (!AlgorithmPayload.contains(std::this_thread::get_id()))
			{
				return false;
			}
			return AlgorithmPayload.at(std::this_thread::get_id()).type() == typeid(T);
		}

	private:
		TArray<TWeakPtr<NavNode>> Siblings{};
		TArray<TArray<EVoxelNavLinkPermissions>> SiblingsPermissions{};
		std::unordered_map<std::thread::id, std::any> AlgorithmPayload{};

	};
}