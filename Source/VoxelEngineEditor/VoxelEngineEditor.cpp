#include "VoxelEngineEditor.h"

#define LOCTEXT_NAMESPACE "VoxelEngineEditor"

DEFINE_LOG_CATEGORY(LogVoxelEngineEditor)
+++++++++++++++








void FVoxelEngineEditor::StartupModule()
{
    UE_LOG(LogVoxelEngineEditor, Warning, TEXT("VoxelEngineEditor: Log Started"));
}

void FVoxelEngineEditor::ShutdownModule()
{
    UE_LOG(LogVoxelEngineEditor, Warning, TEXT("VoxelEngineEditor: Log Ended"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoxelEngineEditor, VoxelEngineEditor);