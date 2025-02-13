// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "VoxelTextureAtlasCollection.h"
#include "VoxelTextureAtlasGenerator.generated.h"

/**
 * 
 */
UCLASS()
class VOXELENGINEEDITOR_API UVoxelTextureAtlasGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "VoxelTextureAtlas", meta = (Keywords = "BeginDrawCanvasToRenderTarget", WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool GenerateTextureAtlas(UObject* WorldContextObject, const FName& TexturesPath, UTextureRenderTarget2D* AtlasRenderTarget, UTexture2D* AtlasTexture, int32 VoxelTextureSize = 32);

	UFUNCTION(BlueprintCallable, Category = "VoxelTextureAtlas", meta = (Keywords = "BeginDrawCanvasToRenderTarget", WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool GenerateTextureAtlasFromArray(UObject* WorldContextObject, const TArray<UTexture2D*>& Textures, UTextureRenderTarget2D* AtlasRenderTarget, UTexture2D* AtlasTexture, int32 VoxelTextureSize = 32);

	static bool GenerateTextureAtlasV2(const FAssetData& VoxelTextureAtlasCollectionAssetData);
};
