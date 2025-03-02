// Fill out your copyright notice in the Description page of Project Settings.


#include "FilePersistenceWorldGenerator.h"
#include "VoxelEngine/VoxelEngine.h"
#include "VoxelWorld.h"

bool UFilePersistenceWorldGenerator::LoadWorld(const FString& Path)
{
    LoadedFilePath = FPaths::ConvertRelativePathToFull(Path);
    FPaths::MakeStandardFilename(LoadedFilePath);

    return true;
}

bool UFilePersistenceWorldGenerator::SaveWorld(AVoxelWorld* VoxelWorld, const FString& Path)
{
    check(VoxelWorld);

    FString PathNormalized = FPaths::ConvertRelativePathToFull(Path);
    FPaths::MakeStandardFilename(PathNormalized);

    IFileManager& FileManager = IFileManager::Get();
    TUniquePtr<FArchive> BinaryFileWriter(FileManager.CreateFileWriter(*PathNormalized));

    if (!BinaryFileWriter)
    {
        UE_LOG(LogVoxelEngine, Error, TEXT("Failed to create File Writer for path %s"), *PathNormalized);
        return false;
    }

    FIntVector WorldSize = VoxelWorld->GetWorldSizeVoxel();

    BinaryFileWriter->Serialize(&WorldSize.X, sizeof(WorldSize.X));
    BinaryFileWriter->Serialize(&WorldSize.Y, sizeof(WorldSize.Y));

    for (int X = 0; X < WorldSize.X; X++)
    {
        for (int Y = 0; Y < WorldSize.Y; Y++)
        {
            for (int Z = 0; Z < WorldSize.Z; Z++)
            {
                Voxel VoxelValue = VoxelWorld->GetVoxel(X, Y, Z);
                VoxelType Type = VoxelValue.VoxelTypeId.load();
                BinaryFileWriter->Serialize(&Type, sizeof(Type));
            }
        }
    }

    bool bClosed = BinaryFileWriter->Close();
    if (!bClosed)
    {
        UE_LOG(LogVoxelEngine, Error, TEXT("Failed to close File Writer for path %s"), *PathNormalized);
    }
    BinaryFileWriter = nullptr;

    UE_LOG(LogVoxelEngine, Display, TEXT("World saved to %s"), *PathNormalized);

    return true;
}

FIntVector2 UFilePersistenceWorldGenerator::GetWantedWorldSizeVoxels() const
{
    TUniquePtr<FArchive> BinaryFileReader;
    if (LoadedFilePath != "")
    {
        IFileManager& FileManager = IFileManager::Get();
        BinaryFileReader = TUniquePtr<FArchive>(FileManager.CreateFileReader(*LoadedFilePath));
        if (!BinaryFileReader)
        {
            UE_LOG(LogVoxelEngine, Error, TEXT("Failed to create File Reader for path %s"), *LoadedFilePath);
        }
    }

    if (!BinaryFileReader)
    {
        return FIntVector2(32, 32);
    }
    
    FIntVector2 WorldSize2D;
    BinaryFileReader->Seek(0);
    BinaryFileReader->Serialize(&WorldSize2D.X, sizeof(WorldSize2D.X));
    BinaryFileReader->Serialize(&WorldSize2D.Y, sizeof(WorldSize2D.Y));
    return WorldSize2D;
}

void UFilePersistenceWorldGenerator::GenerateWorld(AVoxelWorld* VoxelWorld, const FVoxelWorlGenerationFinished& Callback)
{
    TUniquePtr<FArchive> BinaryFileReader;
    if (LoadedFilePath != "")
    {
        IFileManager& FileManager = IFileManager::Get();
        BinaryFileReader = TUniquePtr<FArchive>(FileManager.CreateFileReader(*LoadedFilePath));
        if (!BinaryFileReader)
        {
            UE_LOG(LogVoxelEngine, Error, TEXT("Failed to create File Reader for path %s"), *LoadedFilePath);
        }
    }

    FIntVector WorldSize = VoxelWorld->GetWorldSizeVoxel();
    if (!BinaryFileReader)
    {
        for (int X = 0; X < WorldSize.X; X++)
        {
            for (int Y = 0; Y < WorldSize.Y; Y++)
            {
                VoxelWorld->GetVoxel(X, Y, 0).VoxelTypeId = 3;
            }
        }
        Callback.ExecuteIfBound();
        return;
    }

    BinaryFileReader->Seek(sizeof(int32) * 2);
    
    for (int X = 0; X < WorldSize.X; X++)
    {
        for (int Y = 0; Y < WorldSize.Y; Y++)
        {
            for (int Z = 0; Z < WorldSize.Z; Z++)
            {
                FIntVector VoxelCoord(X, Y, Z);
                VoxelType Type;
                BinaryFileReader->Serialize(&Type, sizeof(Type));
                Voxel& Voxel = VoxelWorld->GetVoxel(VoxelCoord);
                Voxel.VoxelTypeId = Type;
            }
        }
    }
    Callback.ExecuteIfBound();
}

bool UFilePersistenceWorldGenerator::ArePathsEqual(const FString& Path1, const FString& Path2) const
{
    FString FullPath1 = FPaths::ConvertRelativePathToFull(Path1);
    FString FullPath2 = FPaths::ConvertRelativePathToFull(Path2);

    // Standardize the file names to handle different path representations (e.g., backslashes vs forward slashes)
    FPaths::MakeStandardFilename(FullPath1);
    FPaths::MakeStandardFilename(FullPath2);

    // Compare the standardized full paths
    return FullPath1.Equals(FullPath2, ESearchCase::IgnoreCase);
}
