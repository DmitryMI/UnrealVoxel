#include "VoxelEngineTest.h"

#define LOCTEXT_NAMESPACE "VoxelEngineTest"

DEFINE_LOG_CATEGORY(LogVoxelEngineTest)

void FVoxelEngineTest::StartupModule()
{
    UE_LOG(LogVoxelEngineTest, Warning, TEXT("VoxelEngineTest: Log Started"));
}

void FVoxelEngineTest::ShutdownModule()
{
    UE_LOG(LogVoxelEngineTest, Warning, TEXT("VoxelEngineTest: Log Ended"));
}

IMPLEMENT_MODULE(FVoxelEngineTest, VoxelEngineTest);