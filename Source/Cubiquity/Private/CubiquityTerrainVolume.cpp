// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityTerrainVolume.h"
#include "CubiquityOctreeNode.h"
#include "CubiquityMeshComponent.h"

ACubiquityTerrainVolume::ACubiquityTerrainVolume(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	volumeFileName = TEXT("G:/cubiquity/Data/VoxelDatabases/Version 0/SmoothVoxeliensTerrain.vdb");

	/*ConstructorHelpers::FObjectFinder<UMaterial> materialInstance(TEXT("Material'/Cubiquity/Materials/Triplanar5_Inst.Triplanar5_Inst'"));

	if (materialInstance.Succeeded())
	{
		UE_LOG(CubiquityLog, Log, TEXT("Found material"));
	}
	else
	{
		UE_LOG(CubiquityLog, Log, TEXT("Didn't find material"));
	}*/
}

/*ACubiquityTerrainVolume::~ACubiquityTerrainVolume()
{
	//Cubiquity::validate(cuDeleteTerrainVolume(volumeHandle));
}*/

void ACubiquityTerrainVolume::PostActorCreated()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityTerrainVolume::PostActorCreated"));

	volume = loadVolume<Cubiquity::TerrainVolume>();

	const auto eyePosition = eyePositionInVolumeSpace();
	//while (!volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold)) { /*Keep calling update until it returns true*/ }
	volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	createOctree();
	
	Super::PostActorCreated();
}

void ACubiquityTerrainVolume::PostLoad()
{
	//In here, we are loading an existing volume. We should initialise all the Cubiquity stuff by loading the filename from the UProperty
	//It seems too early to spawn actors as the World doesn't exist yet.
	//Actors in the tree will have been serialised anyway so should be loaded.

	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityTerrainVolume::PostLoad"));

	volume = loadVolume<Cubiquity::TerrainVolume>();

	const auto eyePosition = eyePositionInVolumeSpace();
	//while (!volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold)) { /*Keep calling update until it returns true*/ }
	volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	Super::PostLoad();
}

void ACubiquityTerrainVolume::Destroyed()
{
	Super::Destroyed();

	volume.reset(nullptr);
}

void ACubiquityTerrainVolume::createOctree()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityTerrainVolume::loadVolume"));

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

void ACubiquityTerrainVolume::Tick(float DeltaSeconds)
{
	const auto eyePosition = eyePositionInVolumeSpace();
	volume->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	if (octreeRootNodeActor)
	{
		octreeRootNodeActor->processOctreeNode(volume->rootOctreeNode());
	}

	Super::Tick(DeltaSeconds);
}

void ACubiquityTerrainVolume::sculptTerrain(const FVector& worldPosition, float innerRadius, float outerRadius, float opacity)
{
	//Do we really need to check for this being zero?
	if (worldPosition.IsZero())
	{
		return;
	}

	auto voxelPosition = worldToVolume(worldPosition);

	volume->sculpt({ voxelPosition.X, voxelPosition.Y, voxelPosition.Z }, innerRadius, outerRadius, opacity);
}

FVector ACubiquityTerrainVolume::pickSurface(const FVector& start, const FVector& direction)
{
	bool success;
	const FVector localStart = worldToVolume(start);
	const FVector localDirection = worldToVolume(direction);
	auto hitLocation = volume->pickSurface({ localStart.X, localStart.Y, localStart.Z }, { localDirection.X, localDirection.Y, localDirection.Z }, &success);

	if (!success)
	{
		UE_LOG(CubiquityLog, Log, TEXT("Surface pick found nothing"));
		return FVector::ZeroVector;
	}

	return volumeToWorld({ hitLocation.x, hitLocation.y, hitLocation.z });
}

void ACubiquityTerrainVolume::setVoxel(const FIntVector& position, const UCubiquityMaterialSet* materialSet)
{
	volume->setVoxel({ position.X, position.Y, position.Z }, *materialSet);
}

UCubiquityMaterialSet* ACubiquityTerrainVolume::getVoxel(const FIntVector& position)
{
	const auto& voxel = volume->getVoxel({ position.X, position.Y, position.Z });
	return new UCubiquityMaterialSet(voxel);
}
