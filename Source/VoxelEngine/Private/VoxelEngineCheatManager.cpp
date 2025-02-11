// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelEngineCheatManager.h"
#include "VoxelWorld.h"
#include "Kismet/GameplayStatics.h"

void UVoxelEngineCheatManager::DrawChunkWireframes(bool bEnabled)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelWorld::StaticClass(), FoundActors);
	check(FoundActors.Num() == 1);
	AVoxelWorld* VoxelWorld = Cast<AVoxelWorld>(FoundActors[0]);
	VoxelWorld->DrawChunkWireframes(bEnabled);

	if (bEnabled)
	{
		UE_LOG(LogTemp, Display, TEXT("Enabled drawing Voxel Chunks wireframes"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Disabled drawing Voxel Chunks wireframes"));
	}
}

void UVoxelEngineCheatManager::DrawChunkWireframe(int32 ChunkX, int32 ChunkY, bool bEnabled)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelWorld::StaticClass(), FoundActors);
	check(FoundActors.Num() == 1);
	AVoxelWorld* VoxelWorld = Cast<AVoxelWorld>(FoundActors[0]);
	VoxelWorld->DrawChunkWireframe(ChunkX, ChunkY, bEnabled);

	if (bEnabled)
	{
		UE_LOG(LogTemp, Display, TEXT("Enabled drawing Voxel Chunk (%d, %d) wireframe"), ChunkX, ChunkY);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Disabled drawing Voxel Chunk (%d, %d) wireframe"), ChunkX, ChunkY);
	}
}
