// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "VoxelRenderingSettings.h"
#include "VoxelTypeSet.h"
#include <functional>
#include "VoxelTextureAtlasGenerator.generated.h"

/**
 * 
 */
UCLASS()
class VOXELENGINE_API UVoxelTextureAtlasGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	static bool GenerateTextureAtlas(UObject* WorldContextObject, UVoxelTypeSet* VoxelTypes, UVoxelRenderingSettings* RenderingSettings);
	static bool GenerateTextureAtlas(
		UObject* WorldContextObject,
		UVoxelTypeSet* VoxelTypes,
		UVoxelRenderingSettings* RenderingSettings,
		UTextureRenderTarget2D* Atlas,
		const FLinearColor& DefaultColor,
		std::function<TArray<UTexture2D*>(UVoxelData*)> TextureGetter
	);
};
