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

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOctreeSearchPointSingleItemTest, "VoxelEngine.Tests.DataStructures.Octree.SearchPoint.SingleItem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
    bool FOctreeSearchPointSingleItemTest::RunTest(const FString& Parameters)
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

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOctreeSearchPointMultiItemTest, "VoxelEngine.Tests.DataStructures.Octree.SearchPoint.MultiItem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
    bool FOctreeSearchPointMultiItemTest::RunTest(const FString& Parameters)
    {
        FBox GlobalBoundary = FBox(FVector(0, 0, 0), FVector(12, 12, 12));
        TArray<FBox> Items
        {
            FBox(FVector(1, 1, 1), FVector(2, 2, 2)),
            FBox(FVector(0.5, 0.5, 0.5), FVector(1.5, 1.5, 1.5)),
            FBox(FVector(0, 2, 2), FVector(3, 3, 3)),
            FBox(FVector(0, 2, 2), FVector(3, 3, 3)),
            FBox(FVector(0, 0, 0), FVector(4, 4, 4)),
            FBox(FVector(10, 10, 10), FVector(11, 11, 11)),
            FBox(FVector(0, 0, 0), FVector(9.5, 0, 0)),
        };        

        VoxelEngine::DataStructures::TOctree<FBox> Octree(GlobalBoundary, [](const FBox& Item) {return Item; }, 2);

        for (const auto& Item : Items)
        {
            Octree.Insert(Item);
        }

        TArray<FVector> SearchPoints
        {
            FVector(0, 0, 0),
            FVector(1.1, 1.1, 1.1),
            FVector(2, 2, 2),
            FVector(9, 0, 0),
            FVector(9.5, 0, 0),
        };

        for (const auto& SearchPoint : SearchPoints)
        {
            TArray<FBox> ExpectedResult;
            for (const auto& Item : Items)
            {
                if (Item.IsInside(SearchPoint))
                {
                    ExpectedResult.Add(Item);
                }
            }
            auto SearchResult = Octree.Search(SearchPoint);
            
            TestEqual("Result size", SearchResult.size(), ExpectedResult.Num());
            for (const FBox& FoundBox : SearchResult)
            {
                bool bIsInExpectedSet = false;
                for (const FBox& ExpectedBox : ExpectedResult)
                {
                    if (FoundBox == ExpectedBox)
                    {
                        bIsInExpectedSet = true;
                        break;
                    }
                }
                TestTrue("Box in expected-set", bIsInExpectedSet);
            }
        }

        

        return true;
    }

}