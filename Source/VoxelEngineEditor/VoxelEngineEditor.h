#pragma once

#include "Engine.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "UnrealEd.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVoxelEngineEditor, All, All)

class FVoxelEngineEditor : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void AddGenerateVoxelTextureAtlasContextMenuOption();
    void RemoveGenerateVoxelTextureAtlasContextMenuOption();

    static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
    static void ExecuteGenerateVoxelTextureAtlas(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
};