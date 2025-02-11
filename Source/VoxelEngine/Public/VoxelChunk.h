// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelChunk.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOXELENGINE_API UVoxelChunk : public UActorComponent
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

protected:
	// Called when the game starts
	void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	int32 ChunkX = 0;
	UPROPERTY(VisibleAnywhere)
	int32 ChunkY = 0;

	UPROPERTY(EditAnywhere)
	bool bDebugDrawDimensions = true;
};
