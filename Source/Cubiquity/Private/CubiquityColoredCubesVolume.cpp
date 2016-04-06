// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityColoredCubesVolume.h"

#include "CubiquityOctreeNode.h"
#include "CubiquityMeshComponent.h"

ACubiquityColoredCubesVolume::ACubiquityColoredCubesVolume(const FObjectInitializer& PCIP)
    : Super(PCIP)
{
    volumeFileName = FPaths::ConvertRelativePathToFull(FPaths::GamePluginsDir() + "Cubiquity/Dependencies/example-vdb/VoxeliensTerrain.vdb");
}

void ACubiquityColoredCubesVolume::PostActorCreated()
{
    UE_LOG(CubiquityLog, Log, TEXT("ACubiquityColoredCubesVolume::PostActorCreated"));

    Super::PostActorCreated();
}

void ACubiquityColoredCubesVolume::PostLoad()
{
    UE_LOG(CubiquityLog, Log, TEXT("ACubiquityColoredCubesVolume::PostLoad"));

    Super::PostLoad();
}

void ACubiquityColoredCubesVolume::Destroyed()
{
    Super::Destroyed();

    m_volume.reset(nullptr);
}

void ACubiquityColoredCubesVolume::loadVolume()
{
    m_volume = loadVolumeImpl<Cubiquity::ColoredCubesVolume>();
}

void ACubiquityColoredCubesVolume::pickFirstSolidVoxel(const FVector& localStartPosition, const FVector& localDirection, bool& success, FVector& hitLocation) const
{
    auto ex_hitLocation = m_volume->pickFirstSolidVoxel({ localStartPosition.X, localStartPosition.Y, localStartPosition.Z }, { localDirection.X, localDirection.Y, localDirection.Z }, &success);

    if (!success)
    {
        UE_LOG(CubiquityLog, Log, TEXT("Surface pick found nothing"));
        hitLocation = FVector::ZeroVector;
    }
    else
    {
        hitLocation = FVector(ex_hitLocation.x, ex_hitLocation.y, ex_hitLocation.z);
    }
}

void ACubiquityColoredCubesVolume::pickLastEmptyVoxel(const FVector& localStartPosition, const FVector& localDirection, bool& success, FVector& hitLocation) const
{
    auto ex_hitLocation = m_volume->pickLastEmptyVoxel({ localStartPosition.X, localStartPosition.Y, localStartPosition.Z }, { localDirection.X, localDirection.Y, localDirection.Z }, &success);

    if (!success)
    {
        UE_LOG(CubiquityLog, Log, TEXT("Surface pick found nothing"));
        hitLocation = FVector::ZeroVector;
    }
    else
    {
        hitLocation = FVector(ex_hitLocation.x, ex_hitLocation.y, ex_hitLocation.z);
    }
}

void ACubiquityColoredCubesVolume::setVoxel(FVector position, FColor newColor)
{
    m_volume->setVoxel({ position.X, position.Y, position.Z }, { newColor.R, newColor.G, newColor.B, newColor.A });
}

FColor ACubiquityColoredCubesVolume::getVoxel(FVector position) const
{
    const auto& voxel = m_volume->getVoxel({ position.X, position.Y, position.Z });
    return {voxel.red(), voxel.green(), voxel.blue(), voxel.alpha()};
}
