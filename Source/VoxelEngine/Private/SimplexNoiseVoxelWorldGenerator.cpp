// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplexNoiseVoxelWorldGenerator.h"
#include "VoxelEngine/Public/Noise/SimplexNoise.h"
#include "VoxelWorld.h"

FIntVector2 USimplexNoiseVoxelWorldGenerator::GetWantedWorldSizeVoxels() const
{
	return WorldSize;
}

void USimplexNoiseVoxelWorldGenerator::GenerateWorld(AVoxelWorld* VoxelWorld, const FVoxelWorlGenerationFinished& Callback)
{
	check(VoxelWorld);

    UVoxelTypeSet* VoxelTypeSet = VoxelWorld->GetVoxelTypeSet();
    check(VoxelTypeSet);
    VoxelType GrassType = VoxelTypeSet->GetVoxelTypeByName("Grass");
    VoxelType DirtType = VoxelTypeSet->GetVoxelTypeByName("Dirt");
    VoxelType StoneType = VoxelTypeSet->GetVoxelTypeByName("Stone");
    check(GrassType != EmptyVoxelType);
    check(DirtType != EmptyVoxelType);
    check(StoneType != EmptyVoxelType);

	FIntVector WorldSizeActual = VoxelWorld->GetWorldSizeVoxel();
    for (int X = 0; X < WorldSizeActual.X; X++)
    {
        for (int Y = 0; Y < WorldSizeActual.Y; Y++)
        {
            FVector WorldPos2D = VoxelWorld->GetVoxelCenterWorld(FIntVector(X, Y, 0));
            float NoiseValue = VoxelEngine::Noise::SimplexNoise::Noise(WorldPos2D.X * NoiseScale, WorldPos2D.Y * NoiseScale);

            int32 Height = TerrainAverageHeight + NoiseValue * HeightAmplitude;
            int32 DirtLowest = Height - GrassThickness - DirthThickness;
            int32 GrassLowest = Height - GrassThickness;

            for (int Z = 0; Z < WorldSizeActual.Z; Z++)
            {
                FIntVector VoxelCoord(X, Y, Z);
                Voxel& Voxel = VoxelWorld->GetVoxel(VoxelCoord);

                if (Z <= DirtLowest)
                {
                    Voxel.VoxelTypeId = StoneType;
                }
                else if (Z < GrassLowest)
                {
                    Voxel.VoxelTypeId = DirtType;
                }
                else if (Z <= Height)
                {
                    Voxel.VoxelTypeId = GrassType;
                }
                else
                {
                    Voxel.VoxelTypeId = EmptyVoxelType;
                }
            }
        }
    }

    Callback.ExecuteIfBound();
}

