// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplexNoiseVoxelWorldGenerator.h"
#include "SimplexNoise.h"
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
    FVoxelType GrassType = VoxelTypeSet->GetVoxelTypeByName("Grass");
    FVoxelType DirtType = VoxelTypeSet->GetVoxelTypeByName("Dirt");
    FVoxelType StoneType = VoxelTypeSet->GetVoxelTypeByName("Stone");
    check(!GrassType.IsEmptyVoxelType());
    check(!DirtType.IsEmptyVoxelType());
    check(!StoneType.IsEmptyVoxelType());

	FIntVector WorldSizeActual = VoxelWorld->GetWorldSizeVoxel();
    for (int X = 0; X < WorldSizeActual.X; X++)
    {
        for (int Y = 0; Y < WorldSizeActual.Y; Y++)
        {
            FVector WorldPos2D = VoxelWorld->GetVoxelCenterWorld(FIntVector(X, Y, 0));
            float NoiseValue = USimplexNoise::Noise(WorldPos2D.X * NoiseScale, WorldPos2D.Y * NoiseScale);

            int32 Height = TerrainAverageHeight + NoiseValue * HeightAmplitude;
            int32 DirtLowest = Height - GrassThickness - DirthThickness;
            int32 GrassLowest = Height - GrassThickness;

            for (int Z = 0; Z < WorldSizeActual.Z; Z++)
            {
                FIntVector VoxelCoord(X, Y, Z);
                Voxel& Voxel = VoxelWorld->GetVoxel(VoxelCoord);

                if (Z <= DirtLowest)
                {
                    Voxel.VoxelType = StoneType;
                }
                else if (Z < GrassLowest)
                {
                    Voxel.VoxelType = DirtType;
                }
                else if (Z <= Height)
                {
                    Voxel.VoxelType = GrassType;
                }
                else
                {
                    Voxel.VoxelType = 0;
                }
            }
        }
    }

    Callback.ExecuteIfBound();
}

