#pragma once
#include "VoxelEngine/Public/VoxelWorld.h"

namespace VoxelEngine::Query
{
	DECLARE_DELEGATE_RetVal_OneParam(bool, FLineTraceCallback, const FIntVector&);

	class LineTrace
	{
	public:
		static void AmanatidesWooAlgorithm(AVoxelWorld* VoxelWorld, const FVector& Start, const FVector& Direction, double MaxDistance, const FLineTraceCallback& Callback) noexcept;
	private:
		static bool RayBoxIntersection(AVoxelWorld* VoxelWorld, const FVector& Start, const FVector& Direction, double MaxDistance, double& tMin, double& tMax,
			double t0, double t1) noexcept;
	
	};
}