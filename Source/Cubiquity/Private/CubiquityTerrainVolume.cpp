// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityTerrainVolume.h"

#include "CubiquityOctreeNode.h"
#include "CubiquityMeshComponent.h"
#include "CubiquityMaterialSet.h"

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

void ACubiquityTerrainVolume::sculptTerrain(FVector localPosition, float innerRadius, float outerRadius, float opacity)
{
	m_volume->sculpt({ localPosition.X, localPosition.Y, localPosition.Z }, innerRadius, outerRadius, opacity);
}

FVector ACubiquityTerrainVolume::pickSurface(FVector localStartPosition, FVector localDirection)
{
	bool success;
	auto hitLocation = m_volume->pickSurface({ localStartPosition.X, localStartPosition.Y, localStartPosition.Z }, { localDirection.X, localDirection.Y, localDirection.Z }, &success);

	if (!success)
	{
		UE_LOG(CubiquityLog, Log, TEXT("Surface pick found nothing"));
		return FVector::ZeroVector;
	}

	return { hitLocation.x, hitLocation.y, hitLocation.z };
}

void ACubiquityTerrainVolume::setVoxel(FIntVector position, const UCubiquityMaterialSet* materialSet)
{
	m_volume->setVoxel({ position.X, position.Y, position.Z }, *materialSet);
}

UCubiquityMaterialSet* ACubiquityTerrainVolume::getVoxel(FIntVector position)
{
	const auto& voxel = m_volume->getVoxel({ position.X, position.Y, position.Z });
	return new UCubiquityMaterialSet(voxel);
}
