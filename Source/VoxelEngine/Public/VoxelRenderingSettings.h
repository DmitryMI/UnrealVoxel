// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Engine/DataAsset.h"
#include "VoxelRenderingSettings.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class VOXELENGINE_API UVoxelRenderingSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* BaseMaterial = nullptr;

	UPROPERTY(EditAnywhere)
	int32 VoxelTextureSize = 32;

	/// <summary>
	/// Texture file name format:
	/// T_<VoxelName>_<North|South|West|East|Top|Bottom>_<BaseColor|Metallic|Specular|Normal|Emissive>
	/// </summary>
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

	FPrimaryAssetId GetPrimaryAssetId() const override 
	{ 
		FPrimaryAssetId Id = Super::GetPrimaryAssetId();
		return Id;
	}
	

private:
	
};
