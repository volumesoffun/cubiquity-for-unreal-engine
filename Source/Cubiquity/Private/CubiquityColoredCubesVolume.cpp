// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityColoredCubesVolume.h"
#include "CubiquityOctreeNode.h"
#include "CubiquityMeshComponent.h"

ACubiquityColoredCubesVolume::ACubiquityColoredCubesVolume(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	volumeFileName = "G:/cubiquity/Data/VoxelDatabases/Version 0/VoxeliensTerrain.vdb";
}

/*ACubiquityTerrainVolume::~ACubiquityTerrainVolume()
{
	//Cubiquity::validate(cuDeleteTerrainVolume(volumeHandle));
}*/

void ACubiquityColoredCubesVolume::PostActorCreated()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityColoredCubesVolume::PostActorCreated"));
	
	volume = std::make_unique<Cubiquity::ColoredCubesVolume>(TCHAR_TO_ANSI(*volumeFileName), Cubiquity::WritePermissions::ReadOnly, 32);
	const auto eyePosition = eyePositionInVolumeSpace();
	//while (!volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold)) { /*Keep calling update until it returns true*/ }
	volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	loadVolume();
	
	Super::PostActorCreated();
}

void ACubiquityColoredCubesVolume::PostLoad()
{
	//In here, we are loading an existing volume. We should initialise all the Cubiquity stuff by loading the filename from the UProperty
	//It seems too early to spawn actors as the World doesn't exist yet.
	//Actors in the tree will have been serialised anyway so should be loaded.

	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityColoredCubesVolume::PostLoad"));
	
	volume = std::make_unique<Cubiquity::ColoredCubesVolume>(TCHAR_TO_ANSI(*volumeFileName), Cubiquity::WritePermissions::ReadOnly, 32);
	const auto eyePosition = eyePositionInVolumeSpace();
	//while (!volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold)) { /*Keep calling update until it returns true*/ }
	volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	Super::PostLoad();
}

void ACubiquityColoredCubesVolume::Destroyed()
{
	Super::Destroyed();

	volume.reset(nullptr);
}

void ACubiquityColoredCubesVolume::loadVolume()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityColoredCubesVolume::loadVolume"));

	if (volume->hasRootOctreeNode())
	{
		auto rootOctreeNode = volume->rootOctreeNode();

		FActorSpawnParameters spawnParameters;
		spawnParameters.Owner = this;
		octreeRootNodeActor = GetWorld()->SpawnActor<ACubiquityOctreeNode>(spawnParameters);
		octreeRootNodeActor->initialiseOctreeNode(rootOctreeNode, RootComponent, Material);
		octreeRootNodeActor->processOctreeNode(rootOctreeNode);
	}
}

void ACubiquityColoredCubesVolume::Tick(float DeltaSeconds)
{
	const auto eyePosition = eyePositionInVolumeSpace();
	volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	if (octreeRootNodeActor)
	{
		octreeRootNodeActor->processOctreeNode(volume->rootOctreeNode());
	}

	Super::Tick(DeltaSeconds);
}

FVector ACubiquityColoredCubesVolume::pickFirstSolidVoxel(FVector start, FVector direction)
{
	bool success;
	const FVector localStart = worldToVolume(start);
	const FVector localDirection = worldToVolume(direction);
	auto hitLocation = volume->pickFirstSolidVoxel({ localStart.X, localStart.Y, localStart.Z }, { localDirection.X, localDirection.Y, localDirection.Z }, &success);

	if (!success)
	{
		UE_LOG(CubiquityLog, Log, TEXT("Surface pick found nothing"));
		return FVector::ZeroVector;
	}

	return volumeToWorld(FVector(hitLocation.x, hitLocation.y, hitLocation.z));
}

FVector ACubiquityColoredCubesVolume::pickLastEmptyVoxel(FVector start, FVector direction)
{
	bool success;
	const FVector localStart = worldToVolume(start);
	const FVector localDirection = worldToVolume(direction);
	auto hitLocation = volume->pickLastEmptyVoxel({ localStart.X, localStart.Y, localStart.Z }, { localDirection.X, localDirection.Y, localDirection.Z }, &success);

	if (!success)
	{
		UE_LOG(CubiquityLog, Log, TEXT("Surface pick found nothing"));
		return FVector::ZeroVector;
	}

	return volumeToWorld(FVector(hitLocation.x, hitLocation.y, hitLocation.z));
}

void ACubiquityColoredCubesVolume::setVoxel(FIntVector position, FColor newColor)
{
	volume->setVoxel({position.X, position.Y, position.Z}, {newColor.R, newColor.G, newColor.B, newColor.A});
}

FColor ACubiquityColoredCubesVolume::getVoxel(FIntVector position)
{
	const auto& voxel = volume->getVoxel({position.X, position.Y, position.Z});
	return {voxel.red(), voxel.green(), voxel.blue(), voxel.alpha()};
}
