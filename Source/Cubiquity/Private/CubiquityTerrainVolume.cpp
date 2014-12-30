// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityTerrainVolume.h"
#include "CubiquityOctreeNode.h"
#include "CubiquityMeshComponent.h"

ACubiquityTerrainVolume::ACubiquityTerrainVolume(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	volumeFileName = TEXT("G:/cubiquity/Data/VoxelDatabases/Version 0/SmoothVoxeliensTerrain.vdb");
}

void ACubiquityTerrainVolume::PostActorCreated()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityTerrainVolume::PostActorCreated"));
	
	Super::PostActorCreated();
}

void ACubiquityTerrainVolume::PostLoad()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityTerrainVolume::PostLoad"));

	Super::PostLoad();
}

void ACubiquityTerrainVolume::Destroyed()
{
	Super::Destroyed();

	m_volume.reset(nullptr);
}

void ACubiquityTerrainVolume::loadVolume()
{
	m_volume = loadVolumeImpl<Cubiquity::TerrainVolume>();
}

void ACubiquityTerrainVolume::sculptTerrain(const FVector& worldPosition, float innerRadius, float outerRadius, float opacity)
{
	//Do we really need to check for this being zero?
	if (worldPosition.IsZero())
	{
		return;
	}

	auto voxelPosition = worldToVolume(worldPosition);

	m_volume->sculpt({ voxelPosition.X, voxelPosition.Y, voxelPosition.Z }, innerRadius, outerRadius, opacity);
}

FVector ACubiquityTerrainVolume::pickSurface(const FVector& start, const FVector& direction)
{
	bool success;
	const FVector localStart = worldToVolume(start);
	const FVector localDirection = worldToVolume(direction);
	auto hitLocation = m_volume->pickSurface({ localStart.X, localStart.Y, localStart.Z }, { localDirection.X, localDirection.Y, localDirection.Z }, &success);

	if (!success)
	{
		UE_LOG(CubiquityLog, Log, TEXT("Surface pick found nothing"));
		return FVector::ZeroVector;
	}

	return volumeToWorld({ hitLocation.x, hitLocation.y, hitLocation.z });
}

void ACubiquityTerrainVolume::setVoxel(const FIntVector& position, const UCubiquityMaterialSet* materialSet)
{
	m_volume->setVoxel({ position.X, position.Y, position.Z }, *materialSet);
}

UCubiquityMaterialSet* ACubiquityTerrainVolume::getVoxel(const FIntVector& position)
{
	const auto& voxel = m_volume->getVoxel({ position.X, position.Y, position.Z });
	return new UCubiquityMaterialSet(voxel);
}
