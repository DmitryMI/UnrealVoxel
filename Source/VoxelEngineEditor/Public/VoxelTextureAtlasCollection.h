// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "VoxelTextureAtlasCollection.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class VOXELENGINEEDITOR_API UVoxelTextureAtlasCollection : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere)
	bool bTestBool = false;

	UPROPERTY(EditAnywhere)
	FString PathToTexturesDirectory = "/Game/VoxelEngine/Graphics/Textures";

	UPROPERTY(EditAnywhere)
	FString PathToGraphicsDataDirectory = "/Game/VoxelEngine/Graphics/Data";

	UPROPERTY(EditAnywhere)
	FString PathToAtlasDirectory = "/Game/VoxelEngine/Graphics";

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* BaseColorAtlas;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* MetallicAtlas;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* SpecularAtlas;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* NormalAtlas;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* EmissiveAtlas;

	UFUNCTION(CallInEditor, Category="Generating")
	void GenerateAtlasCollection();

private:
	
};
