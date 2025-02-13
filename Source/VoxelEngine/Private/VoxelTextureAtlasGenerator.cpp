// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelTextureAtlasGenerator.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "VoxelEngine/Public/VoxelType.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "VoxelData.h"
#include "Engine/Canvas.h"
#include "VoxelEngine/VoxelEngine.h"
#include "Editor/UnrealEd/Public/ObjectTools.h"

bool UVoxelTextureAtlasGenerator::GenerateTextureAtlas(UObject* WorldContextObject, UVoxelTypeSet* VoxelTypes, UVoxelRenderingSettings* RenderingSettings)
{
	check(VoxelTypes);
	check(RenderingSettings);
	check(RenderingSettings->BaseColorAtlas);
	check(RenderingSettings->MetallicAtlas);
	check(RenderingSettings->SpecularAtlas);
	check(RenderingSettings->EmissiveAtlas);
	check(RenderingSettings->NormalAtlas);

	int32 VoxelTypesNum = VoxelTypes->GetVoxelTypes().Num();

	int32 AtlasSizeX = 6 * RenderingSettings->VoxelTextureSize;
	int32 AtlasSizeY = RenderingSettings->VoxelTextureSize * VoxelTypes->GetVoxelTypes().Num();

	auto BaseColorGetter = [](UVoxelData* VoxelData) {return VoxelData->BaseColor; };
	auto MetallicGetter = [](UVoxelData* VoxelData) {return VoxelData->Metallic; };
	auto SpecularGetter = [](UVoxelData* VoxelData) {return VoxelData->Specular; };
	auto EmissiveGetter = [](UVoxelData* VoxelData) {return VoxelData->Emissive; };
	auto NormalGetter = [](UVoxelData* VoxelData) {return VoxelData->Normal; };

	FLinearColor BaseColorDefault = FLinearColor::Black;
	FLinearColor MetallicDefault = FLinearColor::Black;
	FLinearColor SpecularDefault = FLinearColor::Black;
	FLinearColor EmissiveDefault = FLinearColor(0, 0, 0);
	FLinearColor NormalDefault = FLinearColor(0, 0, 1);

	if (!GenerateTextureAtlas(WorldContextObject, VoxelTypes, RenderingSettings, RenderingSettings->BaseColorAtlas, BaseColorDefault, BaseColorGetter))
	{
		return false;
	}

	if (!GenerateTextureAtlas(WorldContextObject, VoxelTypes, RenderingSettings, RenderingSettings->MetallicAtlas, MetallicDefault, MetallicGetter))
	{
		return false;
	}

	if (!GenerateTextureAtlas(WorldContextObject, VoxelTypes, RenderingSettings, RenderingSettings->SpecularAtlas, SpecularDefault, SpecularGetter))
	{
		return false;
	}

	if (!GenerateTextureAtlas(WorldContextObject, VoxelTypes, RenderingSettings, RenderingSettings->EmissiveAtlas, EmissiveDefault, EmissiveGetter))
	{
		return false;
	}

	if (!GenerateTextureAtlas(WorldContextObject, VoxelTypes, RenderingSettings, RenderingSettings->NormalAtlas, NormalDefault, NormalGetter))
	{
		return false;
	}

	return true;
}

bool UVoxelTextureAtlasGenerator::GenerateTextureAtlas(
	UObject* WorldContextObject,
	UVoxelTypeSet* VoxelTypes,
	UVoxelRenderingSettings* RenderingSettings,
	UTextureRenderTarget2D* Atlas,
	const FLinearColor& DefaultColor,
	std::function<TArray<UTexture2D*>(UVoxelData*)> TextureGetter
)
{
	int32 VoxelTypesNum = VoxelTypes->GetVoxelTypes().Num();

	int32 AtlasSizeX = 6 * RenderingSettings->VoxelTextureSize;
	int32 AtlasSizeY = RenderingSettings->VoxelTextureSize * VoxelTypes->GetVoxelTypes().Num();

	Atlas->ResizeTarget(AtlasSizeX, AtlasSizeY);

	UCanvas* AtlasCanvas;
	FVector2D CanvasSize(AtlasSizeX, AtlasSizeY);
	FDrawToRenderTargetContext DrawContext;
	FVector2D DrawScreenSize(RenderingSettings->VoxelTextureSize, RenderingSettings->VoxelTextureSize);
	FVector2D DrawTextureUvStart = FVector2D(0, 0);
	FVector2D DrawTextureUvSize = FVector2D::UnitVector;
	FLinearColor DrawRenderColor = FLinearColor::White;
	EBlendMode DrawBlendMode = EBlendMode::BLEND_Opaque;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(WorldContextObject, Atlas, AtlasCanvas, CanvasSize, DrawContext);

	TArray<UTexture2D*> VoxelSides;
	VoxelSides.SetNum(6);

	for (int32 VoxelTypeIndex = 0; VoxelTypeIndex < VoxelTypesNum; VoxelTypeIndex++)
	{
		UVoxelData* VoxelData = VoxelTypes->GetVoxelTypes()[VoxelTypeIndex];
		const TArray<UTexture2D*>& VoxelTexturePieces = TextureGetter(VoxelData);
		for (int32 I = 0; I < VoxelTexturePieces.Num(); I++)
		{
			VoxelSides[I] = VoxelTexturePieces[I];
			if (VoxelSides[I]->GetSizeX() != VoxelSides[I]->GetSizeY() || VoxelSides[I]->GetSizeX() != RenderingSettings->VoxelTextureSize)
			{
				UE_LOG(LogVoxelEngine, Error, TEXT("Texture %s has incorrect size (%d, %d). Expected (%d, %d)"),
					*VoxelSides[I]->GetName(),
					VoxelSides[I]->GetSizeX(),
					VoxelSides[I]->GetSizeY(),
					RenderingSettings->VoxelTextureSize,
					RenderingSettings->VoxelTextureSize
				);
				return false;
			}
		}

		if (VoxelTexturePieces.Num() > 0)
		{
			for (int32 I = VoxelTexturePieces.Num(); I < VoxelSides.Num(); I++)
			{
				VoxelSides[I] = VoxelTexturePieces.Last();
			}
		}

		for (int32 I = 0; I < VoxelSides.Num(); I++)
		{
			FVector2D DrawScreenPosition(I * RenderingSettings->VoxelTextureSize, VoxelTypeIndex * RenderingSettings->VoxelTextureSize);
			if (VoxelSides[I])
			{
				AtlasCanvas->K2_DrawTexture(VoxelSides[I], DrawScreenPosition, DrawScreenSize, DrawTextureUvStart, DrawTextureUvSize);
			}
			else
			{
				FIntRect FillRect(
					DrawScreenPosition.X, 
					DrawScreenPosition.Y,
					DrawScreenPosition.X + DrawScreenSize.X,
					DrawScreenPosition.Y + DrawScreenSize.Y
				); 

				FCanvasTileItem TileItem(FVector2D(FillRect.Min), FVector2D(FillRect.Size()), DefaultColor);
				TileItem.BlendMode = SE_BLEND_Opaque;
				AtlasCanvas->DrawItem(TileItem);
			}
		}

	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(WorldContextObject, DrawContext);
	return true;
}
