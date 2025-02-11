// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplexNoise.h"

float USimplexNoise::Noise1(float X)
{
    return Noise(X);
}

float USimplexNoise::Noise2(float X, float Y)
{
    return Noise(X, Y);
}

float USimplexNoise::Noise3(float X, float Y, float Z)
{
    return Noise(X, Y, Z);
}

float USimplexNoise::Fractal1(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X)
{
    return Fractal(Frequency, Amplitude, Lacunarity, Persistence, Octaves, X);
}

float USimplexNoise::Fractal2(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X, float Y)
{
    return Fractal(Frequency, Amplitude, Lacunarity, Persistence, Octaves, X, Y);
}

float USimplexNoise::Fractal3(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X, float Y, float Z)
{
    return Fractal(Frequency, Amplitude, Lacunarity, Persistence, Octaves, X, Y, Z);
}

