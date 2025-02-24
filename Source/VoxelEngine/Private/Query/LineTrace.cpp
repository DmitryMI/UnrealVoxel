#include "VoxelEngine/Public/Query/LineTrace.h"

void VoxelEngine::Query::LineTrace::AmanatidesWooAlgorithm(AVoxelWorld* VoxelWorld, const FVector& Start, const FVector& Direction, double MaxDistance, const FLineTraceCallback& Callback) noexcept
{
	if (!Callback.IsBound())
	{
		return;
	}

	FVector GridMinBound = VoxelWorld->GetActorLocation();
	FVector GridMaxBound = VoxelWorld->GetActorLocation() + FVector(VoxelWorld->GetWorldSizeVoxel()) * VoxelWorld->GetVoxelSizeWorld();

	constexpr double T0 = 0;
	constexpr double T1 = 1;
	double TMin;
	double TMax;
	const bool RayIntersectsGrid = RayBoxIntersection(VoxelWorld, Start, Direction, MaxDistance, TMin, TMax, T0, T1);
	if (!RayIntersectsGrid)
	{
		return;
	}

	TMin = FMath::Max(TMin, T0);
	TMax = FMath::Max(TMax, T1);
	const FVector RayStart = Start + Direction * TMin;
	const FVector RayEnd = Start + Direction * TMax;

	int64 CurrentXIndex = FMath::CeilToInt64((RayStart.X - GridMinBound.X) / VoxelWorld->GetVoxelSizeWorld());
	int64 EndXIndex = FMath::CeilToInt64((RayEnd.X - GridMinBound.X) / VoxelWorld->GetVoxelSizeWorld());
	int StepX;
	double TDeltaX;
	double TMaxX;
	if (Direction.X > 0.0)
	{
		StepX = 1;
		TDeltaX = VoxelWorld->GetVoxelSizeWorld() / Direction.X;
		TMaxX = TMin + (GridMinBound.X + CurrentXIndex * VoxelWorld->GetVoxelSizeWorld()
			- RayStart.X) / Direction.X;
	}
	else if (Direction.X < 0.0)
	{
		StepX = -1;
		TDeltaX = VoxelWorld->GetVoxelSizeWorld() / -Direction.X;
		int64 PreviousXIndex = CurrentXIndex - 1;
		TMaxX = TMin + (GridMinBound.X + PreviousXIndex * VoxelWorld->GetVoxelSizeWorld()
			- RayStart.X) / Direction.X;
	}
	else {
		StepX = 0;
		TDeltaX = TMax;
		TMaxX = TMax;
	}

	int64 CurrentYIndex = FMath::CeilToInt64((RayStart.Y - GridMinBound.Y) / VoxelWorld->GetVoxelSizeWorld());
	int64 EndYIndex = FMath::CeilToInt64((RayEnd.Y - GridMinBound.Y) / VoxelWorld->GetVoxelSizeWorld());
	int StepY;
	double TDeltaY;
	double TMaxY;
	if (Direction.Y > 0.0)
	{
		StepY = 1;
		TDeltaY = VoxelWorld->GetVoxelSizeWorld() / Direction.Y;
		TMaxY = TMin + (GridMinBound.Y + CurrentYIndex * VoxelWorld->GetVoxelSizeWorld()
			- RayStart.Y) / Direction.Y;
	}
	else if (Direction.Y < 0.0)
	{
		StepY = -1;
		TDeltaY = VoxelWorld->GetVoxelSizeWorld() / -Direction.Y;
		int64 PreviousYIndex = CurrentYIndex - 1;
		TMaxY = TMin + (GridMinBound.Y + PreviousYIndex * VoxelWorld->GetVoxelSizeWorld()
			- RayStart.Y) / Direction.Y;
	}
	else
	{
		StepY = 0;
		TDeltaY = TMax;
		TMaxY = TMax;
	}

	int64 CurrentZIndex = FMath::CeilToInt64((RayStart.Z - GridMinBound.Z) / VoxelWorld->GetVoxelSizeWorld());
	int64 EndZIndex = FMath::CeilToInt64((RayEnd.Z - GridMinBound.Z) / VoxelWorld->GetVoxelSizeWorld());
	int StepZ;
	double TDeltaZ;
	double TMaxZ;
	if (Direction.Z > 0.0)
	{
		StepZ = 1;
		TDeltaZ = VoxelWorld->GetVoxelSizeWorld() / Direction.Z;
		TMaxZ = TMin + (GridMinBound.Z + CurrentZIndex * VoxelWorld->GetVoxelSizeWorld()
			- RayStart.Z) / Direction.Z;
	}
	else if (Direction.Z < 0.0)
	{
		StepZ = -1;
		TDeltaZ = VoxelWorld->GetVoxelSizeWorld() / -Direction.Z;
		int64 PreviousZIndex = CurrentZIndex - 1;
		TMaxZ = TMin + (GridMinBound.Z + PreviousZIndex * VoxelWorld->GetVoxelSizeWorld()
			- RayStart.Z) / Direction.Z;
	}
	else
	{
		StepZ = 0;
		TDeltaZ = TMax;
		TMaxZ = TMax;
	}

	while (CurrentXIndex != EndXIndex || CurrentYIndex != EndYIndex || CurrentZIndex != EndZIndex)
	{
		FIntVector CurrentVoxel(CurrentXIndex, CurrentYIndex, CurrentZIndex);
		if (!Callback.Execute(CurrentVoxel))
		{
			return;
		}

		if (TMaxX < TMaxY && TMaxX < TMaxZ) {
			// X-axis traversal.
			CurrentXIndex += StepX;
			TMaxX += TDeltaX;
		}
		else if (TMaxY < TMaxZ) {
			// Y-axis traversal.
			CurrentYIndex += StepY;
			TMaxY += TDeltaY;
		}
		else {
			// Z-axis traversal.
			CurrentZIndex += StepZ;
			TMaxZ += TDeltaZ;
		}
	}

	FIntVector EndVoxel(EndXIndex, EndYIndex, EndZIndex);
	Callback.Execute(EndVoxel);
}

bool VoxelEngine::Query::LineTrace::RayBoxIntersection(AVoxelWorld* VoxelWorld, const FVector& Start, const FVector& Direction, double MaxDistance, double& TMin, double& TMax, double T0, double T1) noexcept
{
	FVector End = Start + Direction * MaxDistance;
	FVector GridMinBound;
	FVector GridMaxBound;
	GridMinBound.X = FMath::Min(Start.X, End.X);
	GridMinBound.Y = FMath::Min(Start.Y, End.Y);
	GridMinBound.Z = FMath::Min(Start.Z, End.Z);

	GridMaxBound.X = FMath::Max(Start.X, End.X);
	GridMaxBound.Y = FMath::Max(Start.Y, End.Y);
	GridMaxBound.Z = FMath::Max(Start.Z, End.Z);

	double TyMin, TyMax, TzMin, TzMax;
	const double XInvDir = 1 / Direction.X;
	if (XInvDir >= 0)
	{
		TMin = (GridMinBound.X - Start.X) * XInvDir;
		TMax = (GridMaxBound.X - Start.X) * XInvDir;
	}
	else
	{
		TMin = (GridMaxBound.X - Start.X) * XInvDir;
		TMax = (GridMinBound.X - Start.X) * XInvDir;
	}

	const double YInvDir = 1 / Direction.Y;
	if (YInvDir >= 0)
	{
		TyMin = (GridMinBound.Y - Start.Y) * YInvDir;
		TyMax = (GridMaxBound.Y - Start.Y) * YInvDir;
	}
	else {
		TyMin = (GridMaxBound.Y - Start.Y) * YInvDir;
		TyMax = (GridMinBound.Y - Start.Y) * YInvDir;
	}

	if (TMin > TyMax || TyMin > TMax) return false;
	if (TyMin > TMin) TMin = TyMin;
	if (TyMax < TMax) TMax = TyMax;

	const double ZInvDir = 1 / Direction.Z;
	if (ZInvDir >= 0)
	{
		TzMin = (GridMinBound.Z - Start.Z) * ZInvDir;
		TzMax = (GridMaxBound.Z - Start.Z) * ZInvDir;
	}
	else
	{
		TzMin = (GridMaxBound.Z - Start.Z) * ZInvDir;
		TzMax = (GridMinBound.Z - Start.Z) * ZInvDir;
	}

	if (TMin > TzMax || TzMin > TMax) return false;
	if (TzMin > TMin) TMin = TzMin;
	if (TzMax < TMax) TMax = TzMax;
	return (TMin < T1 && TMax > T0);
}
