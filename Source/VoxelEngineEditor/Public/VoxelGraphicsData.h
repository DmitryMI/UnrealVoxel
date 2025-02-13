// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelEngine/Public/VoxelType.h"
#include "Engine/Texture2D.h"
#include "VoxelGraphicsData.generated.h"


UCLASS(Blueprintable)
class VOXELENGINEEDITOR_API UVoxelGraphicsData : public UObject
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere)
	bool bDoNotRegenerate = false;

	// Array elements interpretation
	// 0 elements - default value will be used
	// 1 element - will be used for all 6 sides
	// 2 elements - Top, (Bottom + Sides)
	// 3 elements - Top, Bottom, (Front + Back + Left + Right)
	// 4 elements - Top, Bottom, Front, (Back + Left + Right)
	// 5 elements - Top, Bottom, Front, Back, (Left + Right)
	// 6 elements - Top, Bottom, Left, Right, Front, Back
	UPROPERTY(EditAnywhere)
	TArray<UTexture2D*> BaseColor;

	UPROPERTY(EditAnywhere)
	TArray<UTexture2D*> Metallic;

	UPROPERTY(EditAnywhere)
	TArray<UTexture2D*> Specular;

	UPROPERTY(EditAnywhere)
	TArray<UTexture2D*> Normal;

	UPROPERTY(EditAnywhere)
	TArray<UTexture2D*> Emissive;
};
