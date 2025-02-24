#include "Misc/AutomationTest.h"
#include "VoxelEngine/Public/DataStructures/RStarTree.h"

namespace VoxelEngine::DataStructures
{
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRStarTreeConstructionTest, "VoxelEngine.Tests.DataStructures.RStarTree.Ctor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)  
    bool FRStarTreeConstructionTest::RunTest(const FString& Parameters)
    {
        FBox GlobalBoundary = FBox(FVector(0, 0, 0), FVector(1000, 1000, 1000));
        // FBox Box1(FVector(1, 1, 1), FVector(2, 2, 2));
        VoxelEngine::DataStructures::TRStarTree<FBox> RStarTree;
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRStarTreeSearchPointSingleItemTest, "VoxelEngine.Tests.DataStructures.RStarTree.SearchPoint.SingleItem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
    bool FRStarTreeSearchPointSingleItemTest::RunTest(const FString& Parameters)
    {
        FBox GlobalBoundary = FBox(FVector(0, 0, 0), FVector(1000, 1000, 1000));
        FBox Box1(FVector(1, 1, 1), FVector(2, 2, 2));
        VoxelEngine::DataStructures::TRStarTree<FBox> RStarTree;

        RStarTree.Insert(Box1, Box1);
        auto SearchResult = RStarTree.Query(FVector(0, 0, 0));
        TestEqual("Search on miss 1", SearchResult.Num(), 0);

        SearchResult = RStarTree.Query(FVector(3, 3, 3));
        TestEqual("Search on miss 2", SearchResult.Num(), 0);

        SearchResult = RStarTree.Query(FVector(1.1, 1.1, 1.1));
        TestEqual("Search on hit", SearchResult.Num(), 1);
        TestEqual("Search result Key", SearchResult[0].Key, Box1);
        TestEqual("Search result Value", SearchResult[0].Value, Box1);

        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRStarTreeSearchPointMultiItemTest, "VoxelEngine.Tests.DataStructures.RStarTree.SearchPoint.MultiItem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
    bool FRStarTreeSearchPointMultiItemTest::RunTest(const FString& Parameters)
    {
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

        VoxelEngine::DataStructures::TRStarTree<FBox> RStarTree;

        for (const auto& Item : Items)
        {
            RStarTree.Insert(Item, Item);
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
                if (Item.IsInsideOrOn(SearchPoint))
                {
                    ExpectedResult.Add(Item);
                }
            }
            auto SearchResult = RStarTree.Query(SearchPoint);
            
            TestEqual("Result size", SearchResult.Num(), ExpectedResult.Num());
            for (const TPair<FBox, FBox>& FoundBoxPair : SearchResult)
            {
                bool bIsInExpectedSet = false;
                for (const FBox& ExpectedBox : ExpectedResult)
                {
                    TestEqual("Key must equal Value", FoundBoxPair.Key, FoundBoxPair.Value);
                    if (FoundBoxPair.Key == ExpectedBox)
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