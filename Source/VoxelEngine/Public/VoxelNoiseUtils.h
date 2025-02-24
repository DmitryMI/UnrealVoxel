#pragma once

// https://github.com/SRombauts/SimplexNoise/blob/master/src/SimplexNoise.cpp

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <array>
#include "VoxelNoiseUtils.generated.h"

/**
 * 
 */
UCLASS()
class VOXELENGINE_API UVoxelNoiseUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
    static float Noise1(float x);

    UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
    static float Noise2(float x, float y);

    UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
    static float Noise3(float x, float y, float z);

    UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
    static float Fractal1(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X);
    UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
    static float Fractal2(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X, float Y);
    UFUNCTION(BlueprintCallable, Category = "SimplexNoise")
    static float Fractal3(float Frequency, float Amplitude, float Lacunarity, float Persistence, int64 Octaves, float X, float Y, float Z);
};
