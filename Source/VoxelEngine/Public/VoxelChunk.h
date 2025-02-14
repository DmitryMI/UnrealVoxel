// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Components/DynamicMeshComponent.h"
#include "Voxel.h"
#include "VoxelChunk.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOXELENGINE_API UVoxelChunk : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVoxelChunk();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

protected:
	// Called when the game starts
	void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	int32 ChunkX = 0;
	UPROPERTY(VisibleAnywhere)
	int32 ChunkY = 0;

	UPROPERTY(EditAnywhere)
	bool bDebugDrawDimensions = false;

	UPROPERTY(VisibleAnywhere)
	UDynamicMeshComponent* DynamicMeshComponent;

	void GenerateMesh();
	void ProcessVoxels();
	void ProcessVoxel(int32 X, int32 Y, int32 Z);
	bool IsFaceVisible(int32 X, int32 Y, int32 Z) const;
	void AddFaceData(const Voxel& Voxel, int32 X, int32 Y, int32 Z, int FaceIndex);
	int32 GetVoxelLinearIndex(int32 X, int32 Y, int32 Z) const;

	bool CopyVertexColorsToOverlay(
		const FDynamicMesh3& Mesh,
		UE::Geometry::FDynamicMeshColorOverlay& ColorOverlayOut,
		bool bCompactElements
	);
};
