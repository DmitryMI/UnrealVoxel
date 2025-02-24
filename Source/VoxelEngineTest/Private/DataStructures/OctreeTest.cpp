#include "Misc/AutomationTest.h"
#include "VoxelEngine/Public/DataStructures/Octree.h"

namespace VoxelEngine::DataStructures
{
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOctreeConstructionTest, "VoxelEngine.Tests.DataStructures.Octree.Ctor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)  
    bool FOctreeConstructionTest::RunTest(const FString& Parameters)
    {
        FBox GlobalBoundary = FBox(FVector(0, 0, 0), FVector(1000, 1000, 1000));
        // FBox Box1(FVector(1, 1, 1), FVector(2, 2, 2));
        VoxelEngine::DataStructures::TOctree<FBox> Octree(GlobalBoundary, [](const FBox& Item) {return Item; });
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOctreeSingleItemTest, "VoxelEngine.Tests.DataStructures.Octree.SingleItem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
        bool FOctreeSingleItemTest::RunTest(const FString& Parameters)
    {
        FBox GlobalBoundary = FBox(FVector(0, 0, 0), FVector(1000, 1000, 1000));
        FBox Box1(FVector(1, 1, 1), FVector(2, 2, 2));
        VoxelEngine::DataStructures::TOctree<FBox> Octree(GlobalBoundary, [](const FBox& Item) {return Item; });

        Octree.Insert(Box1);
        auto SearchResult = Octree.Search(FVector(0, 0, 0));
        TestEqual("Search on miss 1", SearchResult.size(), 0);

        SearchResult = Octree.Search(FVector(3, 3, 3));
        TestEqual("Search on miss 2", SearchResult.size(), 0);

        SearchResult = Octree.Search(FVector(1.1, 1.1, 1.1));
        TestEqual("Search on hit", SearchResult.size(), 1);

        return true;
    }

}