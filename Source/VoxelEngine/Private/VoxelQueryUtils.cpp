// Based on https://github.com/cgyurgyik/fast-voxel-traversal-algorithm/blob/master/amanatidesWooAlgorithm.cpp

#define VOXEL_OVERLAP_FILTER_DRAW_DEBUG_SHAPES (WITH_EDITOR && 0)

#include "VoxelQueryUtils.h"
#include "VoxelEngine/VoxelEngine.h"
#include "VoxelEngine/Public/Query/LineTrace.h"
#if VOXEL_OVERLAP_FILTER_DRAW_DEBUG_SHAPES
#include "DrawDebugHelpers.h"
#endif

bool UVoxelQueryUtils::VoxelLineTraceFilterSingle(AVoxelWorld* VoxelWorld, const FVector& Start, const FVector& Direction, const FVoxelLineTraceFilterParams& Params, FIntVector& OutHitCoord)
{
	bool bWorldValid = IsValid(VoxelWorld);
	ensureMsgf(bWorldValid, TEXT("VoxelWorld is nullptr"));
	if (!bWorldValid)
	{
		return false;
	}
	
	if (Direction.IsNearlyZero())
	{
		return false;
	}
	FVector DirectionNormalized;
	if (Direction.IsNormalized())
	{
		DirectionNormalized = Direction;
	}
	else
	{
		DirectionNormalized = Direction.GetUnsafeNormal();
	}
	
	ensureMsgf(Params.MaxDistance > 0, TEXT("MaxDistance must be greater then zero"));

#if VOXEL_OVERLAP_FILTER_DRAW_DEBUG_SHAPES
	FVector LineEnd = Start + Params.MaxDistance * DirectionNormalized;
	DrawDebugLine(VoxelWorld->GetWorld(), Start, LineEnd, FColor::Red);
#endif

	FVector StartAdjusted = Start - FVector(VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld());
	
	bool bHasValue = false;

	auto VoxelCallback = [VoxelWorld, Params, &OutHitCoord, &bHasValue](const FIntVector& Voxel)
		{
 			if (CheckIfVoxelSatisfiesLineTraceFilter(VoxelWorld, Voxel, Params))
			{
				OutHitCoord = Voxel;
				bHasValue = true;
				return false;
			}
			return true;
		};

	FAmanatidesWooAlgorithmVoxelCallback Callback = FAmanatidesWooAlgorithmVoxelCallback::CreateLambda(VoxelCallback);

	VoxelEngine::Query::LineTrace::AmanatidesWooAlgorithm(VoxelWorld, StartAdjusted, DirectionNormalized, Params.MaxDistance, Callback);

	return bHasValue;
}

bool UVoxelQueryUtils::VoxelBoxOverlapFilterMulti(AVoxelWorld* VoxelWorld, const FBox& BoxWorld, TArray<FIntVector>& OverlappedVoxels, const FVoxelQueryFilterParams& Params)
{
	bool bWorldValid = IsValid(VoxelWorld);
	ensureMsgf(bWorldValid, TEXT("VoxelWorld is nullptr"));
	if (!bWorldValid)
	{
		return false;
	}

	FBox WorldBoundingBox = VoxelWorld->GetBoundingBoxWorld();
	if (!BoxWorld.Intersect(WorldBoundingBox))
	{
		return false;
	}

	FIntVector MinVoxel = FIntVector(BoxWorld.Min / VoxelWorld->GetVoxelSizeWorld());
	FIntVector MaxVoxel = FIntVector(BoxWorld.Max / VoxelWorld->GetVoxelSizeWorld());

	for (int X = MinVoxel.X; X <= MaxVoxel.X; X++)
	{
		for (int Y = MinVoxel.Y; Y <= MaxVoxel.Y; Y++)
		{
			for (int Z = MinVoxel.Z; Z <= MaxVoxel.Z; Z++)
			{
				FIntVector Coord{ X, Y, Z };
				if (!CheckIfVoxelSatisfiesQueryFilter(VoxelWorld, Coord, Params))
				{
					continue;
				}
				OverlappedVoxels.Add(Coord);
			}
		}
	}

	return OverlappedVoxels.Num() > 0;
}

bool UVoxelQueryUtils::VoxelIntBoxOverlapFilterSingle(AVoxelWorld* VoxelWorld, const FIntBox& VoxelBox, FIntVector& OutHit, const FVoxelQueryFilterParams& Params)
{
	bool bWorldValid = IsValid(VoxelWorld);
	ensureMsgf(bWorldValid, TEXT("VoxelWorld is nullptr"));
	if (!bWorldValid)
	{
		return false;
	}

	FIntVector WorldSize = VoxelWorld->GetWorldSizeVoxel();
	FIntBox WorldVoxelBox(FIntVector{ 0, 0, 0 }, WorldSize);
	if (!WorldVoxelBox.Intersect(VoxelBox))
	{
		return false;
	}

	FIntVector MinVoxel = VoxelBox.Min;
	FIntVector MaxVoxel = VoxelBox.Max;

	for (int X = MinVoxel.X; X <= MaxVoxel.X; X++)
	{
		for (int Y = MinVoxel.Y; Y <= MaxVoxel.Y; Y++)
		{
			for (int Z = MinVoxel.Z; Z <= MaxVoxel.Z; Z++)
			{
				FIntVector Coord{ X, Y, Z };
				if (CheckIfVoxelSatisfiesQueryFilter(VoxelWorld, Coord, Params))
				{
					OutHit = Coord;
					return true;
				}
			}
		}
	}

	return false;
}

bool UVoxelQueryUtils::GetLookedThroughAdjacentVoxel(AVoxelWorld* VoxelWorld, const FIntVector& VoxelCoord, const FVector& RayOrigin, FVector RayDirection, FIntVector& OutAdjacentVoxel)
{
	check(VoxelWorld);
	if (RayDirection.IsNearlyZero())
	{
		return false;
	}
	if (!RayDirection.IsNormalized())
	{
		RayDirection = RayDirection.GetUnsafeNormal();
	}

	FBox VoxelBox = VoxelWorld->GetVoxelBoundingBox(VoxelCoord);
	FVector TMin = (VoxelBox.Min - RayOrigin) / RayDirection;
	FVector TMax = (VoxelBox.Max - RayOrigin) / RayDirection;
	
	FVector T1{ FMath::Min(TMin.X, TMax.X), FMath::Min(TMin.Y, TMax.Y), FMath::Min(TMin.Z, TMax.Z) };
	FVector T2{ FMath::Max(TMin.X, TMax.X), FMath::Max(TMin.Y, TMax.Y), FMath::Max(TMin.Z, TMax.Z) };

	double TEnter = FMath::Max3(T1.X, T1.Y, T1.Z);
	double TExit = FMath::Min3(T2.X, T2.Y, T2.Z);

	if (TEnter <= TExit && TExit >= 0)
	{
		int IndexMax = FMath::Max3Index(T1.X, T1.Y, T1.Z);
		int DirectionSign = -FMath::Sign(RayDirection[IndexMax]);
		FIntVector Offset{ 0, 0, 0 };
		Offset[IndexMax] = DirectionSign;
		OutAdjacentVoxel = VoxelCoord + Offset;
		return true;
	}

	return false;
}

bool UVoxelQueryUtils::CheckIfVoxelSatisfiesQueryFilter(AVoxelWorld* VoxelWorld, const FIntVector& Coord, const FVoxelQueryFilterParams& Params)
{
	if (!VoxelWorld->IsValidCoordinate(Coord))
	{
		return false;
	}

	Voxel& Voxel = VoxelWorld->GetVoxel(Coord);

	if (Voxel.VoxelTypeId == EmptyVoxelType)
	{
#if VOXEL_OVERLAP_FILTER_DRAW_DEBUG_SHAPES
		FVector Location = VoxelWorld->GetVoxelCenterWorld(Coord);
		FVector Extent = FVector(VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld()) / 2;
		DrawDebugBox(VoxelWorld->GetWorld(), Location, Extent, FColor::Cyan);
#endif
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

#if VOXEL_OVERLAP_FILTER_DRAW_DEBUG_SHAPES
	FVector Location = VoxelWorld->GetVoxelCenterWorld(Coord);
	FVector Extent = FVector(VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld(), VoxelWorld->GetVoxelSizeWorld()) / 2;
	DrawDebugBox(VoxelWorld->GetWorld(), Location, Extent, Color);
#endif
	return bPass;

	return false;
}

bool UVoxelQueryUtils::CheckIfVoxelSatisfiesLineTraceFilter(AVoxelWorld* VoxelWorld, const FIntVector& Coord, const FVoxelLineTraceFilterParams& Params)
{
	return CheckIfVoxelSatisfiesQueryFilter(VoxelWorld, Coord, Params);
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
