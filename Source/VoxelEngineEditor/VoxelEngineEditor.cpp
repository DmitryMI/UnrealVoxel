#include "VoxelEngineEditor.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "VoxelEngineEditor/Public/VoxelTextureAtlasCollection.h"

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
    auto NewExtender = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FVoxelEngineEditor::OnExtendContentBrowserAssetSelectionMenu);
    Extenders.Add(NewExtender);
}

void FVoxelEngineEditor::RemoveGenerateVoxelTextureAtlasContextMenuOption()
{
}

TSharedRef<FExtender> FVoxelEngineEditor::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
    UClass* ClassOfInterest = UVoxelTextureAtlasCollection::StaticClass();
    
    for (const FAssetData& AssetData : SelectedAssets)
    {
        UObject* Asset = AssetData.GetAsset();
        if (!Asset)
        {
            UE_LOG(LogVoxelEngineEditor, Error, TEXT("Cannot load asset %s"), *AssetData.AssetName.ToString());
            return MakeShared<FExtender>();
        }
        UBlueprint* BlueprintAsset = Cast<UBlueprint>(Asset);
        if (!BlueprintAsset)
        {
            return MakeShared<FExtender>();
        }
        UClass* AssetClass = BlueprintAsset->ParentClass.Get();
        if (!AssetClass->IsChildOf(ClassOfInterest))
        {
            return MakeShared<FExtender>();
        }
    }

    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    Extender->AddMenuExtension(
        "CommonAssetActions",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateStatic(&FVoxelEngineEditor::ExecuteGenerateVoxelTextureAtlas, SelectedAssets)
    );
    return Extender;
}

void FVoxelEngineEditor::ExecuteGenerateVoxelTextureAtlas(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
    MenuBuilder.BeginSection("Your Asset Context", LOCTEXT("ASSET_CONTEXT", "VoxelEngine"));
    MenuBuilder.AddMenuEntry(
        LOCTEXT("ButtonName", "Generate Voxel Texture Atlas"),
        LOCTEXT("Button ToolTip", "Looks for voxel textures, create VoxelGraphicsData assets and generates Voxel Texture Atlas based on selected VoxelTextureAtlasCollection asset."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([SelectedAssets]() {})),
        NAME_None,
        EUserInterfaceActionType::Button
    );

    MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoxelEngineEditor, VoxelEngineEditor);