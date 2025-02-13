// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorld.h"
#include "Misc/DateTime.h"
#include "SimplexNoise.h"
#include "VoxelTextureAtlasGenerator.h"
#include "VoxelEngine/VoxelEngine.h"

// Sets default values
AVoxelWorld::AVoxelWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AVoxelWorld::DrawChunkWireframes(bool bEnabled)
{
	for (UVoxelChunk* Chunk : Chunks)
	{
		Chunk->SetDrawWireframe(bEnabled);
	}
}

void AVoxelWorld::DrawChunkWireframe(int32 ChunkX, int32 ChunkY, bool bEnabled)
{
	if (0 > ChunkX || ChunkX >= ChunkWorldDimensions.X || 0 > ChunkY || ChunkY >= ChunkWorldDimensions.Y)
	{
		UE_LOG(LogVoxelEngine, Error, TEXT("DrawChunkWireframe failed: invalid chunk index (%d, %d)"), ChunkX, ChunkY);
		return;
	}

	int32 ChunkIndex = ChunkY * ChunkWorldDimensions.X + ChunkX;
	Chunks[ChunkIndex]->SetDrawWireframe(bEnabled);
}

FIntVector AVoxelWorld::GetWorldSizeVoxel() const
{
	return FIntVector(ChunkWorldDimensions.X * ChunkSide, ChunkWorldDimensions.Y * ChunkSide, WorldHeight);
}

// Called when the game starts or when spawned
void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	if (!InitializeMaterials())
	{
		return;
	}

	if (!VoxelWorldGeneratorClass)
	{
		UE_LOG(LogVoxelEngine, Error, TEXT("Voxel World Generator class not set!"));
		return;
	}

	VoxelWorldGeneratorInstance = NewObject<UVoxelWorldGenerator>(this, VoxelWorldGeneratorClass, FName("VoxelWorldGeneratorInstance"));
	check(VoxelWorldGeneratorInstance);

	FIntVector2 GeneratorWantsSize = VoxelWorldGeneratorInstance->GetWantedWorldSizeVoxels();
	int32 ChunksX = GeneratorWantsSize.X / ChunkSide;
	int32 ChunksY = GeneratorWantsSize.Y / ChunkSide;
	if (GeneratorWantsSize.X % ChunkSide != 0)
	{
		ChunksX += 1;
	}
	if (GeneratorWantsSize.Y % ChunkSide != 0)
	{
		ChunksY += 1;
	}
	ChunkWorldDimensions = FIntVector2(ChunksX, ChunksY);

	UE_LOG(LogVoxelEngine, Display, TEXT("Allocating Voxel World memory..."));
	FDateTime AllocStartTime = FDateTime::Now();
	size_t Length = ChunkWorldDimensions.X * ChunkSide;
	size_t Width = ChunkWorldDimensions.Y * ChunkSide;
	size_t Height = WorldHeight;
	size_t VoxelsNum = Length * Width * Height;
	Voxels.resize(VoxelsNum);
	FDateTime AllocEndTime = FDateTime::Now();
	FTimespan AllocElapsedTime = AllocEndTime - AllocStartTime;
	UE_LOG(LogVoxelEngine, Display, TEXT("Voxel World memory allocated, %d voxels in total, %3.2f milliseconds"), Voxels.size(), AllocElapsedTime.GetTotalMilliseconds());

	UVoxelWorldGenerator::FVoxelWorlGenerationFinished Callback;
	Callback.BindUFunction(this, FName("WorldGenerationFinishedCallback"));
	VoxelWorldGeneratorInstance->GenerateWorld(this, Callback);
}

void AVoxelWorld::WorldGenerationFinishedCallback()
{
	UE_LOG(LogVoxelEngine, Display, TEXT("Spawning Chunk components..."));
	FDateTime ChunkSpawnStartTime = FDateTime::Now();
	for (int32 X = 0; X < ChunkWorldDimensions.X; X++)
	{
		for (int32 Y = 0; Y < ChunkWorldDimensions.Y; Y++)
		{
			UVoxelChunk* Chunk = NewObject<UVoxelChunk>(this);
			check(Chunk);
			Chunk->SetChunkIndex(X, Y);

			Chunk->RegisterComponent();
			FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, false);
			Chunk->AttachToComponent(RootComponent, Rules);
			AddOwnedComponent(Chunk);
			Chunks.Add(Chunk);
		}
	}
	FDateTime ChunkSpawnEndTime = FDateTime::Now();
	FTimespan ChunkSpawnElapsedTime = ChunkSpawnEndTime - ChunkSpawnStartTime;
	UE_LOG(LogVoxelEngine, Display, TEXT("Spawned %d Chunk components, %3.2f milliseconds"), ChunkWorldDimensions.X * ChunkWorldDimensions.Y, ChunkSpawnElapsedTime.GetTotalMilliseconds());
}

bool AVoxelWorld::InitializeMaterials()
{
	if (!RenderingSettings)
	{
		UE_LOG(LogVoxelEngine, Error, TEXT("InitializeMaterials() failed: RenderingSettings is nullptr"));
		return false;
	}

	if (!RenderingSettings->BaseMaterial)
	{
		UE_LOG(LogVoxelEngine, Error, TEXT("InitializeMaterials() failed: RenderingSettings->BaseMaterial is nullptr"));
		return false;
	}

	if (!UVoxelTextureAtlasGenerator::GenerateTextureAtlas(this, VoxelTypeSet, RenderingSettings))
	{
		UE_LOG(LogVoxelEngine, Error, TEXT("InitializeMaterials() failed: Atlas creation error."));
		return false;
	}

	DynamicMaterialInstance = UMaterialInstanceDynamic::Create(RenderingSettings->BaseMaterial, this);
	DynamicMaterialInstance->SetTextureParameterValue("BaseColorAtlas", RenderingSettings->BaseColorAtlas);
	DynamicMaterialInstance->SetTextureParameterValue("MetallicAtlas", RenderingSettings->MetallicAtlas);
	DynamicMaterialInstance->SetTextureParameterValue("SpecularAtlas", RenderingSettings->SpecularAtlas);
	DynamicMaterialInstance->SetTextureParameterValue("EmissiveAtlas", RenderingSettings->EmissiveAtlas);
	DynamicMaterialInstance->SetTextureParameterValue("NormalAtlas", RenderingSettings->NormalAtlas);
	return true;
}

// Called every frame
void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

uint64 AVoxelWorld::LinearizeCoordinate(int32 X, int32 Y, int32 Z) const
{
	checkSlow(0 <= X && X <= ChunkWorldDimensions.X * ChunkSide);
	checkSlow(0 <= Y && Y <= ChunkWorldDimensions.Y * ChunkSide);
	checkSlow(0 <= Z && Z <= WorldHeight);

	uint64 Result = 
		static_cast<size_t>(Z * ChunkWorldDimensions.X * ChunkSide * ChunkWorldDimensions.Y * ChunkSide) +
		static_cast<size_t>(Y * (ChunkWorldDimensions.X * ChunkSide)) +
		static_cast<size_t>(X);
	check(Result < Voxels.size());
	return Result;
}

FIntVector AVoxelWorld::DelinearizeCoordinate(int32 LinearCoord) const
{
	int32 X = LinearCoord % (ChunkWorldDimensions.X * ChunkSide);
	int32 Y = (LinearCoord / (ChunkWorldDimensions.X * ChunkSide)) % (ChunkWorldDimensions.Y * ChunkSide);
	int32 Z = LinearCoord / ((ChunkWorldDimensions.X * ChunkSide) * (ChunkWorldDimensions.Y * ChunkSide));
	return FIntVector(X, Y, Z);
}

const Voxel& AVoxelWorld::GetVoxel(const FIntVector& Coord) const
{
	size_t Index = LinearizeCoordinate(Coord.X, Coord.Y, Coord.Z);
	return Voxels[Index];
}

Voxel& AVoxelWorld::GetVoxel(const FIntVector& Coord)
{
	return GetVoxel(Coord.X, Coord.Y, Coord.Z);
}

Voxel& AVoxelWorld::GetVoxel(int32 X, int32 Y, int32 Z)
{
	size_t Index = LinearizeCoordinate(X, Y, Z);
	return Voxels[Index];
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

bool AVoxelWorld::IsVoxelTransparent(const FIntVector& Coord) const
{
	if (Coord.X < 0 || Coord.X >= ChunkWorldDimensions.X * ChunkSide)
	{
		return true;
	}

	if (Coord.Y < 0 || Coord.Y >= ChunkWorldDimensions.Y * ChunkSide)
	{
		return true;
	}

	if (Coord.Z < 0 || Coord.Z >= WorldHeight)
	{
		return true;
	}

	const Voxel& Voxel = GetVoxel(Coord);
	if (Voxel.VoxelType.VoxelTypeId == FVoxelType::EmptyVoxelType)
	{
		return true;
	}

	UVoxelData* VoxelData = VoxelTypeSet->GetVoxelDataByType(Voxel.VoxelType);
	check(VoxelData);
	return VoxelData->bIsTransparent;
}

UMaterialInterface* AVoxelWorld::GetVoxelChunkMaterial() const
{
	return DynamicMaterialInstance;
}

FVector AVoxelWorld::GetVoxelCenterWorld(const FIntVector& Coord) const
{
	return GetActorLocation() + VoxelSizeWorld * FVector(Coord.X, Coord.Y, Coord.Z) * 1.5;
}

UVoxelTypeSet* AVoxelWorld::GetVoxelTypeSet() const
{
	return VoxelTypeSet;
}

