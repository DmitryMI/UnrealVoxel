// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorld.h"
#include "VoxelChunk.h"

// Sets default values
AVoxelWorld::AVoxelWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Display, TEXT("Allocating Voxel World memory..."));
	Voxels.SetNum(ChunkWorldDimensions.X * ChunkWorldDimensions.Y * ChunkSide * WorldHeight);
	UE_LOG(LogTemp, Display, TEXT("Voxel World memory allocated, %d voxels in total"), Voxels.Num());

	UE_LOG(LogTemp, Display, TEXT("Spawning Chunk components..."));
	for (int32 X = 0; X < ChunkWorldDimensions.X; X++)
	{
		for (int32 Y = 0; Y < ChunkWorldDimensions.Y; Y++)
		{
			UVoxelChunk* Chunk = NewObject<UVoxelChunk>(this);
			check(Chunk);
			Chunk->RegisterComponent();  // This makes it live in the world
			Chunk->SetChunkIndex(X, Y);
		}
	}
	UE_LOG(LogTemp, Display, TEXT("Spawned %d Chunk components"), ChunkWorldDimensions.X * ChunkWorldDimensions.Y);
}

// Called every frame
void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int32 AVoxelWorld::LinearizeCoordinate(int32 X, int32 Y, int32 Z)
{
	return Z * ((ChunkWorldDimensions.X * ChunkSide) * (ChunkWorldDimensions.Y * ChunkSide)) + Y * (ChunkWorldDimensions.X * ChunkSide) + X;
}

FIntVector AVoxelWorld::DelinearizeCoordinate(int32 LinearCoord)
{
	int32 X = LinearCoord % (ChunkWorldDimensions.X * ChunkSide);
	int32 Y = (LinearCoord / (ChunkWorldDimensions.X * ChunkSide)) % (ChunkWorldDimensions.Y * ChunkSide);
	int32 Z = LinearCoord / ((ChunkWorldDimensions.X * ChunkSide) * (ChunkWorldDimensions.Y * ChunkSide));
	return FIntVector(X, Y, Z);
}

Voxel& AVoxelWorld::GetVoxel(const FIntVector& Coord)
{
	return GetVoxel(Coord.X, Coord.Y, Coord.Z);
}

Voxel& AVoxelWorld::GetVoxel(int32 X, int32 Y, int32 Z)
{
	return Voxels[LinearizeCoordinate(X, Y, Z)];
}

void AVoxelWorld::GetChunkWorldDimensions(int32& OutX, int32& OutY) const
{
	OutX = ChunkWorldDimensions.X;
	OutY = ChunkWorldDimensions.Y;
}

int32 AVoxelWorld::GetChunkSide() const
{
	return ChunkSide;
}

int32 AVoxelWorld::GetWorldHeight() const
{
	return WorldHeight;
}

double AVoxelWorld::GetVoxelSizeWorld() const
{
	return VoxelSizeWorld;
}

