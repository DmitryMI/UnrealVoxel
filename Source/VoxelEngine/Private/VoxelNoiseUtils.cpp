// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelNoiseUtils.h"
#include "VoxelEngine/Public/Noise/SimplexNoise.h"

float UVoxelNoiseUtils::Noise1(float X)
{
    return VoxelEngine::Noise::SimplexNoise::Noise(X);
}

float UVoxelNoiseUtils::Noise2(float X, float Y)
{
    return VoxelEngine::Noise::SimplexNoise::Noise(X, Y);
}

float UVoxelNoiseUtils::Noise3(float X, float Y, float Z)
{
    return VoxelEngine::Noise::SimplexNoise::Noise(X, Y, Z);
}

float UVoxelNoiseUtils::Fractal1(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X)
{
    return VoxelEngine::Noise::SimplexNoise::Fractal(Frequency, Amplitude, Lacunarity, Persistence, Octaves, X);
}

float UVoxelNoiseUtils::Fractal2(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X, float Y)
{
    return VoxelEngine::Noise::SimplexNoise::Fractal(Frequency, Amplitude, Lacunarity, Persistence, Octaves, X, Y);
}

float UVoxelNoiseUtils::Fractal3(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X, float Y, float Z)
{
    return VoxelEngine::Noise::SimplexNoise::Fractal(Frequency, Amplitude, Lacunarity, Persistence, Octaves, X, Y, Z);
}

