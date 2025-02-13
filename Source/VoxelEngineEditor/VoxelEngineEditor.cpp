#include "VoxelEngineEditor.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Engine/AssetManager.h"

#define LOCTEXT_NAMESPACE "VoxelEngineEditor"

DEFINE_LOG_CATEGORY(LogVoxelEngineEditor)

void FVoxelEngineEditor::StartupModule()
{
    UE_LOG(LogVoxelEngineEditor, Warning, TEXT("VoxelEngineEditor: Log Started"));

    // IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    // AssetTools.RegisterAssetTypeActions
    AddGenerateVoxelTextureAtlasContextMenuOption();
}

void FVoxelEngineEditor::ShutdownModule()
{
    UE_LOG(LogVoxelEngineEditor, Warning, TEXT("VoxelEngineEditor: Log Ended"));
    RemoveGenerateVoxelTextureAtlasContextMenuOption();
}

void FVoxelEngineEditor::AddGenerateVoxelTextureAtlasContextMenuOption()
{
    FContentBrowserModule& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
    auto& Extenders = ContentBrowser.GetAllAssetViewContextMenuExtenders();
    auto NewExtender = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FVoxelEngineEditor::ContentBrowserAssetSelectionMenuExtenderCallback);
    Extenders.Add(NewExtender);
}

void FVoxelEngineEditor::RemoveGenerateVoxelTextureAtlasContextMenuOption()
{
}

TSharedRef<FExtender> FVoxelEngineEditor::ContentBrowserAssetSelectionMenuExtenderCallback(const TArray<FAssetData>& SelectedAssets)
{
    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    /*
    UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
    UClass* ClassOfInterest = UVoxelTextureAtlasCollection::StaticClass();

    for (const FAssetData& AssetData : SelectedAssets)
    {
        TArray<FSoftObjectPath> CollectionPath{ AssetData.GetSoftObjectPath() };
        auto Handle = AssetManager->LoadAssetList(CollectionPath);
        if (!Handle)
        {
            UE_LOG(LogVoxelEngineEditor, Error, TEXT("Failed to load asset %s"), *AssetData.GetSoftObjectPath().ToString());
            return Extender;
        }
        Handle->WaitUntilComplete();
        UObject* Asset = Handle->GetLoadedAsset();
        if (!Asset)
        {
            UE_LOG(LogVoxelEngineEditor, Error, TEXT("Failed to load asset %s"), *AssetData.GetSoftObjectPath().ToString());
            Handle->ReleaseHandle();
            return Extender;
        }
        UVoxelTextureAtlasCollection* AtlasCollection = Cast<UVoxelTextureAtlasCollection>(Asset);
        if (!AtlasCollection)
        {
            Handle->ReleaseHandle();
            return Extender;
        }

        Handle->ReleaseHandle();
    }

    Extender->AddMenuExtension(
        "CommonAssetActions",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateStatic(&FVoxelEngineEditor::MenuExtenderCallback, SelectedAssets)
    );
    */
    return Extender;
}

void FVoxelEngineEditor::MenuExtenderCallback(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
    MenuBuilder.BeginSection("Your Asset Context", LOCTEXT("ASSET_CONTEXT", "VoxelEngine"));
    MenuBuilder.AddMenuEntry(
        LOCTEXT("ButtonName", "Generate Voxel Texture Atlas"),
        LOCTEXT("Button ToolTip", "Looks for voxel textures, create VoxelGraphicsData assets and generates Voxel Texture Atlas based on selected VoxelTextureAtlasCollection asset."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateStatic(&FVoxelEngineEditor::ExecuteGenerateVoxelTextureAtlasContextMenuAction, SelectedAssets)),
        NAME_None,
        EUserInterfaceActionType::Button
    );

    MenuBuilder.EndSection();
}

void FVoxelEngineEditor::ExecuteGenerateVoxelTextureAtlasContextMenuAction(TArray<FAssetData> SelectedAssets)
{ 
    for (const auto& AssetData : SelectedAssets)
    {
        // UVoxelTextureAtlasGenerator::GenerateTextureAtlasV2(AssetData);
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoxelEngineEditor, VoxelEngineEditor);