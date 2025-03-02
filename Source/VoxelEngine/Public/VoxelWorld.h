// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Voxel.h"
#include "VoxelChunk.h"
#include "VoxelWorldGenerator.h"
#include <vector>
#include "VoxelTypeSet.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VoxelRenderingSettings.h"
#include "VoxelChange.h"
#include "VoxelNavManagerComponent.h"
#include "VoxelWorld.generated.h"

class AVoxelWorld;

USTRUCT()
struct VOXELENGINE_API FVoxelWorldSecondaryTickFunction : public FActorTickFunction
{
	GENERATED_USTRUCT_BODY()

	class AVoxelWorld* Target;

	virtual void ExecuteTick(
		float DeltaTime,
		ELevelTick TickType,
		ENamedThreads::Type CurrentThread,
		const FGraphEventRef& MyCompletionGraphEvent) override;

	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FVoxelWorldSecondaryTickFunction> : public TStructOpsTypeTraitsBase2<FVoxelWorldSecondaryTickFunction>
{
	enum
	{
		WithCopy = false
	};
};


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

	UFUNCTION(BlueprintCallable)
	void RegenerateChunkMeshes();

	UFUNCTION(BlueprintCallable)
	void ResetWorld();

	UFUNCTION(BlueprintCallable)
	FIntVector GetWorldSizeVoxel() const;

	UFUNCTION(BlueprintCallable)
	FBox GetBoundingBoxWorld() const;

	UFUNCTION(BlueprintCallable)
	void GetChunkWorldDimensions(int32& OutX, int32& OutY) const;

	UFUNCTION(BlueprintCallable)
	int32 GetChunkSide() const;

	UFUNCTION(BlueprintCallable)
	int32 GetWorldHeight() const;

	UFUNCTION(BlueprintCallable)
	double GetVoxelSizeWorld() const;

	bool IsVoxelTransparent(const FIntVector& Coord) const;

	bool IsVoxelTransparentTypeOverride(const FIntVector& Coord, VoxelType VoxelType) const;

	bool IsVoxelTraversable(const FIntVector& Coord) const;

	UFUNCTION(BlueprintCallable)
	UMaterialInterface* GetVoxelChunkMaterial() const;

	UFUNCTION(BlueprintCallable)
	FVector GetVoxelCenterWorld(const FIntVector& Coord) const;

	UFUNCTION(BlueprintCallable)
	FBox GetVoxelBoundingBox(const FIntVector& Coord) const;

	UFUNCTION(BlueprintCallable)
	FIntVector GetVoxelCoordFromWorld(const FVector& Location) const;

	UFUNCTION(BlueprintCallable)
	UVoxelTypeSet* GetVoxelTypeSet() const;

	void Tick(float DeltaTime) override;
	virtual void TickSecondary(float DeltaTime, ELevelTick LevelTick, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent, FVoxelWorldSecondaryTickFunction* TickFunction);

	uint64 LinearizeCoordinate(int32 X, int32 Y, int32 Z) const;
	FIntVector DelinearizeCoordinate(uint64 LinearCoord) const;

	uint64 LinearizeChunkCoordinate(const FIntVector2& ChunkCoord) const;
	FIntVector2 DelinearizeChunkCoordinate(uint64 LinearCoord) const;

	FIntVector2 GetChunkCoordFromVoxelCoord(const FIntVector& Coord) const;
	UVoxelChunk* GetChunkFromVoxelCoord(const FIntVector& Coord) const;

	const Voxel& GetVoxel(const FIntVector& Coord) const;

	Voxel& GetVoxel(const FIntVector& Coord);

	Voxel& GetVoxel(int32 X, int32 Y, int32 Z);

	bool IsValidCoordinate(const FIntVector& Coord) const;

	// Thread-safe and lock-free way to change voxel type
	EVoxelChangeResult ChangeVoxel(FVoxelChange& VoxelChange);

	// Thread-safe and lock-free way to change voxel type. Exposed to Blueprints.
	UFUNCTION(BlueprintCallable)
	EVoxelChangeResult ChangeVoxel(const FIntVector& Coord, int32 DesiredVoxelType);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;

private:
	UPROPERTY(EditDefaultsOnly)
	int32 ChunkSide = 32;

	UPROPERTY(EditDefaultsOnly)
	int32 WorldHeight = 32;

	UPROPERTY(EditDefaultsOnly)
	double VoxelSizeWorld = 100;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UVoxelWorldGenerator> VoxelWorldGeneratorClass;

	UPROPERTY(EditDefaultsOnly)
	UVoxelTypeSet* VoxelTypeSet = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UVoxelRenderingSettings* RenderingSettings = nullptr;

	UPROPERTY(VisibleAnywhere)
	TArray<UVoxelChunk*> Chunks;

	UPROPERTY(VisibleAnywhere, Category = Tick)
	FVoxelWorldSecondaryTickFunction SecondaryActorTick;

	UPROPERTY()
	FIntVector2 ChunkWorldDimensions = FIntVector2(4, 4);

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterialInstance = nullptr;

	UPROPERTY()
	UVoxelWorldGenerator* VoxelWorldGeneratorInstance = nullptr;

	UPROPERTY(VisibleAnywhere)
	UVoxelNavManagerComponent* VoxelNavManagerComponent = nullptr;

	std::vector<Voxel> Voxels;

	UFUNCTION()
	void WorldGenerationFinishedCallback();

	UFUNCTION()
	void NavGenerationFinishedCallback();

	bool InitializeMaterials();

};
