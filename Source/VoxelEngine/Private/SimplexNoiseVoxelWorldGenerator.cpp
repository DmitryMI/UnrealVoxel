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
	FIntVector WorldSizeActual = VoxelWorld->GetWorldSizeVoxel();
    for (int X = 0; X < WorldSizeActual.X; X++)
    {
        for (int Y = 0; Y < WorldSizeActual.Y; Y++)
        {
            for (int Z = 0; Z < WorldSizeActual.Z; Z++)
            {
                FIntVector VoxelCoord(X, Y, Z);
                FVector WorldPos = VoxelWorld->GetVoxelCenterWorld(VoxelCoord);
                Voxel& Voxel = VoxelWorld->GetVoxel(VoxelCoord);
                float NoiseValue = USimplexNoise::Noise(WorldPos.X * NoiseScale, WorldPos.Y * NoiseScale);
                int32 Height = TerrainAverageHeight + NoiseValue * HeightAmplitude;
                if (Z < Height)
                {
                    Voxel.VoxelType = EVoxelType::Grass;
                }
                else
                {
                    Voxel.VoxelType = EVoxelType::Air;
                }
            }
        }
    }

    Callback.ExecuteIfBound();
}

