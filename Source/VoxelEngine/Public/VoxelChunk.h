// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Components/DynamicMeshComponent.h"
#include "Voxel.h"
#include "Containers/List.h"
#include "VoxelChange.h"
#include "Containers/BitArray.h"
#include "VoxelChunk.generated.h"

USTRUCT()
struct VOXELENGINE_API FVoxelChunkSecondaryTickFunction : public FActorComponentTickFunction
{
	GENERATED_USTRUCT_BODY()

	class UVoxelChunk* Target;

	virtual void ExecuteTick(
		float DeltaTime,
		ELevelTick TickType,
		ENamedThreads::Type CurrentThread,
		const FGraphEventRef& MyCompletionGraphEvent) override;

	virtual FString DiagnosticMessage() override;
};

template<>
struct TStructOpsTypeTraits<FVoxelChunkSecondaryTickFunction> : public TStructOpsTypeTraitsBase2<FVoxelChunkSecondaryTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOXELENGINE_API UVoxelChunk : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVoxelChunk();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void TickSecondary(float DeltaTime, ELevelTick LevelTick, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent, FVoxelChunkSecondaryTickFunction* TickFunction);

	UFUNCTION(BlueprintCallable)
	int32 GetChunkSide() const;

	UFUNCTION(BlueprintCallable)
	void GetChunkIndex(int32& OutX, int32& OutY) const;
	void SetChunkIndex(int32 X, int32 Y);

	UFUNCTION(BlueprintCallable)
	FBox GetWorldBoundingBox() const;

	UFUNCTION(BlueprintCallable)
	void GetVoxelBoundingBox(FIntVector& OutMin, FIntVector& OutMax) const;

	UFUNCTION(BlueprintCallable)
	void SetDrawWireframe(bool bEnabled);

	UFUNCTION(BlueprintCallable)
	bool GetDrawWireframe() const;

	void MarkMeshDirty();

	EVoxelChangeResult ChangeVoxelRendering(const FVoxelChange& VoxelChange);


protected:
	// Called when the game starts
	void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	bool bDebugDrawDimensions = false;
	
	UPROPERTY(EditDefaultsOnly)
	int32 RenderingMaxDelayFrames = 10;

	UPROPERTY(VisibleAnywhere)
	int32 ChunkX = 0;
	UPROPERTY(VisibleAnywhere)
	int32 ChunkY = 0;

	UPROPERTY(VisibleAnywhere, Category = Tick)
	FVoxelChunkSecondaryTickFunction SecondaryComponentTick;

	UPROPERTY(VisibleAnywhere)
	bool bIsMeshDirty = false;

	UPROPERTY(VisibleAnywhere)
	UDynamicMeshComponent* DynamicMeshComponent;

	// TDoubleLinkedList<int32> VisibleVoxelIndices;
	TBitArray<FDefaultBitArrayAllocator> VisibleVoxelIndices;
	TQueue<FVoxelChange, EQueueMode::Mpsc> VoxelChangeRequests;

	void GenerateMesh();
	void ProcessVoxels();
	void ProcessVoxel(int32 X, int32 Y, int32 Z);
	bool IsFaceVisible(int32 X, int32 Y, int32 Z) const;
	void AddFaceData(const Voxel& Voxel, int32 X, int32 Y, int32 Z, int FaceIndex);
	int32 LinearizeCoordinate(int32 X, int32 Y, int32 Z) const;
	FIntVector DelinearizeCoordinate(int32 LinearCoord) const;
	void ProcessChangeRequest(const FVoxelChange& Request);

	bool CopyVertexColorsToOverlay(
		const FDynamicMesh3& Mesh,
		UE::Geometry::FDynamicMeshColorOverlay& ColorOverlayOut,
		bool bCompactElements
	);

	int CheckVoxelSidesVisibility(const FIntVector& VoxelWorldCoord, TStaticArray<bool, 6>& SideVisilityFlags);
	void UpdateVoxelVisibility(const FIntVector& VoxelWorldCoord, bool bUpdateNeighbours);
	void RegenerateMesh();
};
