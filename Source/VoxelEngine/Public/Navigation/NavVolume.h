#pragma once

#include "CoreMinimal.h"
#include "VoxelEngine/Public/DataStructures/IntBox.h"

namespace VoxelEngine::Navigation
{
	class NavVolume
	{
	public:
		FIntBox VoxelBounds;
		FBox Box;
		TArray<NavVolume*> LinkedVolumes;

		bool Intersect(const NavVolume& Other);
		FIntBox Overlap(const NavVolume& Other);

		void Unlink()
		{
			for (auto* LinkedVolume : LinkedVolumes)
			{
				LinkedVolume->LinkedVolumes.Remove(this);
			}

			LinkedVolumes.Empty();
		}

		void Unlink(NavVolume* Other)
		{
			Other->LinkedVolumes.Remove(this);
			LinkedVolumes.Remove(Other);
		}

		bool TryLink(NavVolume* Other)
		{
			if (!Intersect(*Other))
			{
				return false;
			}

			if (!LinkedVolumes.Contains(Other))
			{
				LinkedVolumes.Add(Other);
			}

			if (!Other->LinkedVolumes.Contains(this))
			{
				Other->LinkedVolumes.Add(this);
			}

			return true;
		}
	};
}