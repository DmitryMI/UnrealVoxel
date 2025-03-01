#pragma once

#include "CoreMinimal.h"

namespace VoxelEngine::DataStructures
{
	template<typename T>
	struct TIntBox
	{
		UE::Math::TIntVector3<T> Min{ 0, 0, 0 };
		UE::Math::TIntVector3<T> Max{ 0, 0, 0 };

		TIntBox()
		{

		}

		TIntBox(const UE::Math::TIntVector3<T>& Min, const UE::Math::TIntVector3<T>& Max) : Min(Min), Max(Max)
		{

		}

		bool IsValid() const
		{
			return Min.X <= Max.X && Min.Y <= Max.Y && Min.Z <= Max.Z;
		}

		bool Intersect(const TIntBox<T>& Other) const
		{
			if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
			{
				return false;
			}

			if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
			{
				return false;
			}

			if ((Min.Z > Other.Max.Z) || (Other.Min.Z > Max.Z))
			{
				return false;
			}

			return true;
		}

		TIntBox<T> Overlap(const TIntBox<T>& Other)
		{
			if (!Intersect(Other))
			{
				return TIntBox<T>();
			}

			UE::Math::TIntVector3<T> MinVector, MaxVector;

			MinVector.X = FMath::Max(Min.X, Other.Min.X);
			MaxVector.X = FMath::Min(Max.X, Other.Max.X);

			MinVector.Y = FMath::Max(Min.Y, Other.Min.Y);
			MaxVector.Y = FMath::Min(Max.Y, Other.Max.Y);

			MinVector.Z = FMath::Max(Min.Z, Other.Min.Z);
			MaxVector.Z = FMath::Min(Max.Z, Other.Max.Z);

			return TIntBox<T>(MinVector, MaxVector);
		}

		UE::Math::TIntVector3<T> GetSize() const
		{
			return UE::Math::TIntVector3<T>(Max.X - Min.X + 1, Max.Y - Min.Y + 1, Max.Z - Min.Z + 1);
		}

		T GetVolume() const
		{
			UE::Math::TIntVector3<T> Size = GetSize();
			return Size.X * Size.Y * Size.Z;
		}

		FBox ToBox(double VoxelSize) const
		{
			FVector MinF = FVector(Min) * VoxelSize;
			FVector MaxF = FVector(Max + UE::Math::TIntVector3<T>{1, 1, 1})* VoxelSize;
			return FBox(MinF, MaxF);
		}
	};
}

using FIntBox = VoxelEngine::DataStructures::TIntBox<int32>;