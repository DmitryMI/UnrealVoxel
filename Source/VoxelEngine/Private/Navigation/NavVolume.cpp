#include "NavVolume.h"

bool VoxelEngine::Navigation::NavVolume::Intersect(const NavVolume& Other)
{
    return VoxelBounds.Intersect(Other.VoxelBounds);
}

FIntBox VoxelEngine::Navigation::NavVolume::Overlap(const NavVolume& Other)
{
    return VoxelBounds.Overlap(Other.VoxelBounds);
}
