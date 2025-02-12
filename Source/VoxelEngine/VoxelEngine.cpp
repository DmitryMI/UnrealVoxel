#include "VoxelEngine.h"

#define LOCTEXT_NAMESPACE "VoxelEngine"

DEFINE_LOG_CATEGORY(LogVoxelEngine)

void FVoxelEngine::StartupModule()
{
    UE_LOG(LogVoxelEngine, Warning, TEXT("VoxelEngine: Log Started"));
}

void FVoxelEngine::ShutdownModule()
{
    UE_LOG(LogVoxelEngine, Warning, TEXT("VoxelEngine: Log Ended"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoxelEngine, VoxelEngine);