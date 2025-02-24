#include "Misc/AutomationTest.h"
#include "VoxelEngine/Public/DataStructures/Octree.h"

namespace VoxelEngine::DataStructures
{
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOctreeConstructionTest, "VoxelEngine.Tests.DataStructures.Octree", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)

    bool FOctreeConstructionTest::RunTest(const FString& Parameters)
    {
        FBox GlobalBoundary = FBox(FVector(0, 0, 0), FVector(1000, 1000, 1000));
        // FBox Box1(FVector(1, 1, 1), FVector(2, 2, 2));
        VoxelEngine::DataStructures::TOctree<FBox> Octree(GlobalBoundary, [](const FBox& Item) {return Item; });
        return true;
    }

}