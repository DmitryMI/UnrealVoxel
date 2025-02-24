// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelChange.h"
#include "VoxelEngine/Public/Navigation/NavVolume.h"
#include "VoxelEngine/Public/DataStructures/RStarTree.h"
#include "VoxelNavManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VOXELENGINE_API UVoxelNavManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	DECLARE_DYNAMIC_DELEGATE(FVoxelNavGenerationFinished);

	UVoxelNavManagerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GenerateNavData(const FVoxelNavGenerationFinished& Callback);
	void ChangeVoxel(const FVoxelChange& VoxelChange);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	FVoxelNavGenerationFinished GenerationFinishedCallback;

	TDoubleLinkedList<TUniquePtr<VoxelEngine::Navigation::NavVolume>> NavVolumes;
	VoxelEngine::DataStructures::TRStarTree<VoxelEngine::Navigation::NavVolume*> NavVolumesRTree;
};
