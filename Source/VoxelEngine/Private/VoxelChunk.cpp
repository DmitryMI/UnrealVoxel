// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UVoxelChunk::UVoxelChunk()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVoxelChunk::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UVoxelChunk::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDebugDrawDimensions)
	{
		AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
		check(VoxelWorld);
		FBox3d Bbox = GetWorldBoundingBox();
		DrawDebugBox(GetWorld(), Bbox.GetCenter(), Bbox.GetExtent(), FColor::Green);
	}
}

int32 UVoxelChunk::GetChunkSide() const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	return VoxelWorld->GetChunkSide();
}

void UVoxelChunk::GetChunkIndex(int32& OutX, int32& OutY) const
{
	OutX = ChunkX;
	OutY = OutY;
}

void UVoxelChunk::SetChunkIndex(int32 X, int32 Y)
{
	ChunkX = X;
	ChunkY = Y;
}

FBox UVoxelChunk::GetWorldBoundingBox() const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	int32 ChunkSide = VoxelWorld->GetChunkSide();
	double VoxelSizeWorld = VoxelWorld->GetVoxelSizeWorld();
	int32 FirstBlockX = ChunkX * ChunkSide;
	int32 FirstBlockY = ChunkY * ChunkSide;
	double WorldX = FirstBlockX * VoxelSizeWorld;
	double WorldY = FirstBlockY * VoxelSizeWorld;
	double WorldZ = 0;

	FVector Min = VoxelWorld->GetActorLocation() + FVector(WorldX, WorldY, WorldZ);
	FVector Dim = FVector(ChunkSide * VoxelSizeWorld, ChunkSide * VoxelSizeWorld, VoxelSizeWorld * VoxelWorld->GetWorldHeight());
	FVector Max = Min + Dim;
	return FBox3d(Min, Max);
}

