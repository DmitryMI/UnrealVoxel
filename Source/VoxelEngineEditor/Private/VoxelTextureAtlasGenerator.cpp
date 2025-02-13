// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelTextureAtlasGenerator.h"
#include "VoxelEngineEditor/VoxelEngineEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "VoxelEngine/Public/VoxelType.h"
#include "Kismet/KismetRenderingLibrary.h"

bool UVoxelTextureAtlasGenerator::GenerateTextureAtlas(UObject* WorldContextObject, const FName& TexturesPath, UTextureRenderTarget2D* AtlasRenderTarget, UTexture2D* AtlasTexture, int32 VoxelTextureSize)
{
	if (!AtlasTexture)
	{
		UE_LOG(LogVoxelEngineEditor, Error, TEXT("Atlas Texture is nullptr"));
		return false;
	}
		
	const UEnum* VoxelTypeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EVoxelType"), true);
	check(VoxelTypeEnum);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Add(TexturesPath);

	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	UE_LOG(LogVoxelEngineEditor, Display, TEXT("Found %d assets in %s"), AssetData.Num(), *TexturesPath.ToString());

	TArray<UTexture2D*> Textures;
	Textures.SetNum(static_cast<int32>(EVoxelType::MAX));

	for (const FAssetData& Asset : AssetData)
	{
		FString Path = Asset.PackageName.ToString();
		UE_LOG(LogVoxelEngineEditor, Display, TEXT("Processing Asset %s..."), *Path);
		UObject* Object = Asset.GetAsset();
		UTexture2D* Texture = Cast<UTexture2D>(Object);
		check(Texture);
		int32 SizeX = Texture->GetImportedSize().X;
		int32 SizeY = Texture->GetImportedSize().Y;
		if ((SizeX != 6 * VoxelTextureSize) || (SizeY != VoxelTextureSize))
		{
			UE_LOG(LogVoxelEngineEditor, Error, TEXT("Texture %s has invalid dimensions: (%d, %d). Expected to be (%d, %d)"), *Path, SizeX, SizeY, 6 * VoxelTextureSize, VoxelTextureSize);
			return false;
		}
		FString AssetName = Texture->GetName();
		if (!AssetName.RemoveFromStart("T_"))
		{
			UE_LOG(LogVoxelEngineEditor, Error, TEXT("Voxel Texture must have a name in format 'T_VoxelType'. Texture %s has malformed name."), *Path);
			return false;
		}

		
		int64 VoxelTypeInt = VoxelTypeEnum->GetValueByNameString(AssetName);
		if (VoxelTypeInt == INDEX_NONE)
		{
			UE_LOG(LogVoxelEngineEditor, Error, TEXT("Failure during processing Voxel Texture %d: Voxel Type %s does not exist."), *Path, *AssetName);
			return false;
		}

		Textures[VoxelTypeInt] = Texture;
	}

	return GenerateTextureAtlasFromArray(WorldContextObject, Textures, AtlasRenderTarget, AtlasTexture, VoxelTextureSize);
}

bool UVoxelTextureAtlasGenerator::GenerateTextureAtlasFromArray(UObject* WorldContextObject, const TArray<UTexture2D*>& Textures, UTextureRenderTarget2D* AtlasRenderTarget, UTexture2D* AtlasTexture, int32 VoxelTextureSize)
{
	check(AtlasTexture);

	int32 AtlasSizeX = 6 * VoxelTextureSize;
	int32 AtlasSizeY = VoxelTextureSize * Textures.Num();
	UE_LOG(LogVoxelEngineEditor, Display, TEXT("Resizing Render Target %s to (%d, %d) to accommodate %d voxel textures"), *AtlasTexture->GetName(), AtlasSizeX, AtlasSizeY, Textures.Num());
	AtlasRenderTarget->ResizeTarget(AtlasSizeX, AtlasSizeY);

	UCanvas* AtlasCanvas;
	FVector2D CanvasSize(AtlasSizeX, AtlasSizeY);
	FDrawToRenderTargetContext DrawContext;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(WorldContextObject, AtlasRenderTarget, AtlasCanvas, CanvasSize, DrawContext);

	for (int32 Index = 0; Index < Textures.Num(); Index++)
	{
		UTexture2D* Texture = Textures[Index];
		if (!Texture)
		{
			continue;
		}

		int32 SizeX = Texture->GetImportedSize().X;
		int32 SizeY = Texture->GetImportedSize().Y;
		FVector2D ScreenSize(SizeX, SizeY);
		FVector2D ScreenPosition(0, Index * VoxelTextureSize);
		FVector2D TextureUvStart = FVector2D(0, 0);
		FVector2D TextureUvSize = FVector2D::UnitVector;
		FLinearColor RenderColor = FLinearColor::White;
		EBlendMode BlendMode = EBlendMode::BLEND_Opaque;
		float Rotation = 0;
		FVector2D PivotPoint(0.5, 0.5);
		AtlasCanvas->K2_DrawTexture(Texture, ScreenPosition, ScreenSize, TextureUvStart, TextureUvSize, RenderColor, BlendMode, Rotation, PivotPoint);
		UE_LOG(LogVoxelEngineEditor, Display, TEXT("Drawing finished for %s at Atlas Position (%3.2f, %3.2f)"), *Texture->GetName(), ScreenPosition.X, ScreenPosition.Y);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(WorldContextObject, DrawContext);

	if (AtlasTexture)
	{
		AtlasRenderTarget->UpdateTexture2D(AtlasTexture, ETextureSourceFormat::TSF_RGBA16);
		UE_LOG(LogVoxelEngineEditor, Display, TEXT("Atlas Texture %s updated"), *AtlasTexture->GetName());
	}
	return true;
}
