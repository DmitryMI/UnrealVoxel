// https://github.com/cgyurgyik/fast-voxel-traversal-algorithm/blob/master/amanatidesWooAlgorithm.cpp

#include "VoxelQueryUtils.h"
#include "VoxelEngine/VoxelEngine.h"
#include "DrawDebugHelpers.h"

bool UVoxelQueryUtils::VoxelLineTraceFilterSingle(AVoxelWorld* VoxelWorld, const FVector& Start, const FVector& Direction, const FVoxelLineTraceFilterParams& Params, FIntVector& OutHitCoord)
{
	bool bWorldValid = IsValid(VoxelWorld);
	ensureMsgf(bWorldValid, TEXT("VoxelWorld is nullptr"));
	if (!bWorldValid)
	{
		return false;
	}
	double VoxelWorldSize = VoxelWorld->GetVoxelSizeWorld();

	FIntVector CurrentVoxel = VoxelWorld->GetVoxelCoordFromWorld(Start);
	if (Params.bIncludeInitialVoxel && CheckIfVoxelSatisfiesFilter(VoxelWorld, CurrentVoxel, Params))
	{
		OutHitCoord = CurrentVoxel;
		return true;
	}
	FVector FirstVoxelCenterWorld = VoxelWorld->GetVoxelCenterWorld(CurrentVoxel);
	FIntVector PreviousVoxel = CurrentVoxel;
	FVector PreviousVoxelCenterWorld = VoxelWorld->GetVoxelCenterWorld(PreviousVoxel);
	FVector DebugVoxelExtent = FVector(VoxelWorldSize, VoxelWorldSize, VoxelWorldSize) / 2;
	DrawDebugBox(VoxelWorld->GetWorld(), FirstVoxelCenterWorld, DebugVoxelExtent, FColor::Emerald, false, -1, 0, 4);

	FVector DirectionNormalized = Direction.GetSafeNormal();
	FVector EndPoint = Start + DirectionNormalized * Params.MaxDistance;
	FIntVector EndVoxel = VoxelWorld->GetVoxelCoordFromWorld(EndPoint);

	DrawDebugBox(VoxelWorld->GetWorld(), VoxelWorld->GetVoxelCenterWorld(EndVoxel), DebugVoxelExtent, FColor::Magenta, false, -1, 0, 4);

	FIntVector Step{ 0, 0, 0 };
	FVector Delta{ 0, 0, 0 };
	FVector NextIntersection{ 0, 0, 0 };
	TStaticArray<bool, 3> DirectionValid{ false, false, false };

	if (DirectionNormalized.X > 0)
	{
		Step.X = 1;
		Delta.X = VoxelWorldSize / DirectionNormalized.X;
		NextIntersection.X = (FirstVoxelCenterWorld.X - Start.X) / DirectionNormalized.X;
		DirectionValid[0] = true;
	}
	else if(DirectionNormalized.X < 0)
	{
		Step.X = -1;
		Delta.X = VoxelWorldSize / -DirectionNormalized.X;
		NextIntersection.X = (PreviousVoxelCenterWorld.X - Start.X) / DirectionNormalized.X;
		DirectionValid[0] = true;
	}

	if (DirectionNormalized.Y > 0)
	{
		Step.Y = 1;
		Delta.Y = VoxelWorldSize / DirectionNormalized.Y;
		NextIntersection.Y = (FirstVoxelCenterWorld.Y - Start.Y) / DirectionNormalized.Y;
		DirectionValid[1] = true;
	}
	else if (DirectionNormalized.Y < 0)
	{
		Step.Y = -1;
		Delta.Y = VoxelWorldSize / -DirectionNormalized.Y;
		NextIntersection.Y = (PreviousVoxelCenterWorld.Y - Start.Y) / DirectionNormalized.Y;
		DirectionValid[1] = true;
	}

	if (DirectionNormalized.Z > 0)
	{
		Step.Z = 1;
		Delta.Z = VoxelWorldSize / DirectionNormalized.Z;
		NextIntersection.Z = (FirstVoxelCenterWorld.Z - Start.Z) / DirectionNormalized.Z;
		DirectionValid[2] = true;
	}
	else if (DirectionNormalized.Z < 0)
	{
		Step.Z = -1;
		Delta.Z = VoxelWorldSize / -DirectionNormalized.Z;
		NextIntersection.Z = (PreviousVoxelCenterWorld.Z - Start.Z) / DirectionNormalized.Z;
		DirectionValid[2] = true;
	}

	bool bIsValidCoordinate = VoxelWorld->IsValidCoordinate(CurrentVoxel);
	while (VoxelWorld->IsValidCoordinate(CurrentVoxel) && (Params.MaxDistance <= 0 || CurrentVoxel != EndVoxel))
	{
		TArray<int> MinComponents = GetMinComponent(NextIntersection,
			{ 
				DirectionNormalized.X != 0,
				DirectionNormalized.Y != 0,
				DirectionNormalized.Z != 0
			}
		);

		for (int I : MinComponents)
		{
			if (I == 0)
			{
				CurrentVoxel.X += Step.X;
				NextIntersection.X += Delta.X;
			}
			else if (I == 1)
			{
				CurrentVoxel.Y += Step.Y;
				NextIntersection.Y += Delta.Y;
			}
			else if (I == 2)
			{
				CurrentVoxel.Z += Step.Z;
				NextIntersection.Z += Delta.Z;
			}
			break;
		}

		if (!CheckIfVoxelSatisfiesFilter(VoxelWorld, CurrentVoxel, Params))
		{
			continue;
		}

		OutHitCoord = CurrentVoxel;
		return true;
	}

	return false;
}

bool UVoxelQueryUtils::CheckIfVoxelSatisfiesFilter(AVoxelWorld* VoxelWorld, const FIntVector& Coord, const FVoxelLineTraceFilterParams& Params)
{
	if (!VoxelWorld->IsValidCoordinate(Coord))
	{
		return false;
	}
	Voxel& Voxel = VoxelWorld->GetVoxel(Coord);
	if (Voxel.VoxelTypeId == EmptyVoxelType)
	{
		FVector Location = VoxelWorld->GetVoxelCenterWorld(Coord);
		FVector Extent = FVector(VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld()) / 2;
		DrawDebugBox(VoxelWorld->GetWorld(), Location, Extent, FColor::Cyan);
		return false;
	}

	UVoxelData* Data = VoxelWorld->GetVoxelTypeSet()->GetVoxelDataByType(Voxel.VoxelTypeId);

	bool bPositivePass = true;
	bool bNegativePass = true;
	if (Params.Traversible == EVoxelLineTraceFilterMode::Positive)
	{
		bPositivePass &= Data->bIsTraversable;
	}
	else if (Params.Traversible == EVoxelLineTraceFilterMode::Negative)
	{
		bNegativePass &= !Data->bIsTraversable;
	}

	if (Params.Transparent == EVoxelLineTraceFilterMode::Positive)
	{
		bPositivePass &= Data->bIsTransparent;
	}
	else if (Params.Transparent == EVoxelLineTraceFilterMode::Negative)
	{
		bNegativePass &= !Data->bIsTransparent;
	}

	bool bPass = bPositivePass && bNegativePass;
	FColor Color;
	if (bPass)
	{
		Color = FColor::Green;
	}
	else
	{
		Color = FColor::Red;
	}

	FVector Location = VoxelWorld->GetVoxelCenterWorld(Coord);
	FVector Extent = FVector(VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld()) / 2;
	DrawDebugBox(VoxelWorld->GetWorld(), Location, Extent, Color);
	return bPass;
}

TArray<int> UVoxelQueryUtils::GetMinComponent(const FVector& Values, const TStaticArray<bool, 3>& ValidityFlags)
{
	int MinIndex = -1;
	TArray<int> Result;
	for (int I = 0; I < 3; I++)
	{
		if (!ValidityFlags[I])
		{
			continue;
		}

		if (MinIndex == -1)
		{
			MinIndex = I;
			Result.Add(I);
			continue;
		}
		if (Values[I] == Values[MinIndex])
		{
			Result.Add(I);
			continue;
		}
		if (Values[I] < Values[MinIndex])
		{
			Result.Empty();
			MinIndex = I;
			Result.Add(I);
			continue;
		}

	}
	return Result;
}
