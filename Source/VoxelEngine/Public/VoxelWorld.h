// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Voxel.h"
#include "VoxelChunk.h"
#include "VoxelWorld.generated.h"

UCLASS()
class VOXELENGINE_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
	
public:	
	AVoxelWorld();

	UFUNCTION(BlueprintCallable)
	void DrawChunkWireframes(bool bEnabled);

	UFUNCTION(BlueprintCallable)
	void DrawChunkWireframe(int32 ChunkX, int32 ChunkY, bool bEnabled);

	virtual void Tick(float DeltaTime) override;

	int32 LinearizeCoordinate(int32 X, int32 Y, int32 Z) const;
	FIntVector DelinearizeCoordinate(int32 LinearCoord) const;

	const Voxel& GetVoxel(const FIntVector& Coord) const;

	Voxel& GetVoxel(const FIntVector& Coord);

	Voxel& GetVoxel(int32 X, int32 Y, int32 Z);

	UFUNCTION(BlueprintCallable)
	void GetChunkWorldDimensions(int32& OutX, int32& OutY) const;
	
	UFUNCTION(BlueprintCallable)
	int32 GetChunkSide() const;
	
	UFUNCTION(BlueprintCallable)
	int32 GetWorldHeight() const;

	UFUNCTION(BlueprintCallable)
	double GetVoxelSizeWorld() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsVoxelTransparent(const FIntVector& Coord) const;

	UFUNCTION(BlueprintCallable)
	UMaterialInterface* GetVoxelChunkMaterial() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	FIntVector2 ChunkWorldDimensions = FIntVector2(4, 4);

	UPROPERTY(EditDefaultsOnly)
	int32 ChunkSide = 16;

	UPROPERTY(EditDefaultsOnly)
	int32 WorldHeight = 16;

	UPROPERTY(EditDefaultsOnly)
	double VoxelSizeWorld = 100;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* VoxelChunkMaterial = nullptr;

	UPROPERTY(VisibleAnywhere)
	TArray<UVoxelChunk*> Chunks;

	TArray<Voxel> Voxels;

};
