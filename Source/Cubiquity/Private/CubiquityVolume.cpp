// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityVolume.h"

#include "CubiquityOctreeNode.h"
#include "CubiquityMeshComponent.h"
#include "CubiquityUpdateComponent.h"

ACubiquityVolume::ACubiquityVolume(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	UE_LOG(CubiquityLog, Log, TEXT("Creating ACubiquityVolume"));
	root = PCIP.CreateDefaultSubobject<UCubiquityUpdateComponent>(this, TEXT("Updater node"));
	RootComponent = root;
	//AddOwnedComponent(root);

	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;
	//PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void ACubiquityVolume::PostActorCreated()
{
	loadVolume();

	const auto eyePosition = eyePositionInVolumeSpace();
	//while (!volume()->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, 0.0)) { /*Keep calling update until it returns true*/ }
	volume()->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	createOctree();

	Super::PostActorCreated();
}

void ACubiquityVolume::PostLoad()
{
	//In here, we are loading an existing volume. We should initialise all the Cubiquity stuff by loading the filename from the UProperty
	//It seems too early to spawn actors as the World doesn't exist yet.
	//Actors in the tree will have been serialised anyway so should be loaded.

	loadVolume();

	const auto eyePosition = eyePositionInVolumeSpace();
	//while (!volume()->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, 0.0)) { /*Keep calling update until it returns true*/ }
	volume()->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	Super::PostLoad();
}

void ACubiquityVolume::OnConstruction(const FTransform& transform)
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityVolume::OnConstruction"));

	if (RootComponent->GetNumChildrenComponents() == 0) //If we haven't created the octree yet
	{
		createOctree();
	}

	updateMaterial();

	Super::OnConstruction(transform);
}

void ACubiquityVolume::PostInitializeComponents()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityVolume::PostInitializeComponents"));

	Super::PostInitializeComponents();
}

void ACubiquityVolume::BeginPlay()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityVolume::BeginPlay"));

	createOctree();

	Super::BeginPlay();
}

/*void Serialize(FArchive& Ar)
{

}*/

void ACubiquityVolume::Destroyed()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityVolume::Destroyed"));

	TArray<AActor*> children = Children; //Make a copy to avoid overruns
	//UE_LOG(CubiquityLog, Log, TEXT(" Children %d"), children.Num());
	for (AActor* childActor : children) //Should only be 1 child of this Actor
	{
		//UE_LOG(CubiquityLog, Log, TEXT("  Destroying childActor"));
		if (childActor && !childActor->IsPendingKillPending())
		{
			GetWorld()->DestroyActor(childActor);
		}
		
	}

	Super::Destroyed();
}

void ACubiquityVolume::processOctree()
{
	const auto eyePosition = eyePositionInVolumeSpace();
	volume()->update({ eyePosition.X, eyePosition.Y, eyePosition.Z }, lodThreshold);

	if (octreeRootNodeActor)
	{
		octreeRootNodeActor->processOctreeNode(volume()->rootOctreeNode(), 1);
	}
}

#if WITH_EDITOR
void ACubiquityVolume::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == FName(TEXT("volumeFileName")))
	{
		//Should we save the old volume? Probably not without asking.
		//Unload old volume
		//Load new one

		TArray<AActor*> children = Children; //Make a copy to avoid overruns
		for (AActor* childActor : children) //Should only be 1 child of this Actor
		{
			if (childActor && !childActor->IsPendingKillPending())
			{
				GetWorld()->DestroyActor(childActor);
			}

		}

		loadVolume();

		createOctree();

		updateMaterial(); //TODO needed?
	}
	else if (PropertyName == FName(TEXT("Material")))
	{
		updateMaterial();
	}
}
#endif

void ACubiquityVolume::createOctree()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityColoredCubesVolume::loadVolume"));

	if (volume()->hasRootOctreeNode())
	{
		auto rootOctreeNode = volume()->rootOctreeNode();

		const FVector childNodeVolumePosition = FVector(rootOctreeNode.position().x, rootOctreeNode.position().y, rootOctreeNode.position().z);

		FActorSpawnParameters spawnParameters;
		spawnParameters.Owner = this;
		octreeRootNodeActor = GetWorld()->SpawnActor<ACubiquityOctreeNode>(childNodeVolumePosition, FRotator::ZeroRotator, spawnParameters);
		octreeRootNodeActor->initialiseOctreeNode(rootOctreeNode, Material);
		//octreeRootNodeActor->processOctreeNode(rootOctreeNode, 1);
	}
}

void ACubiquityVolume::updateMaterial()
{
	TArray<USceneComponent*> children;
	root->GetChildrenComponents(true, children); //Get all children and grandchildren...
	for (USceneComponent* childNode : children)
	{
		UCubiquityMeshComponent* mesh = Cast<UCubiquityMeshComponent>(childNode);
		if (mesh)
		{
			mesh->SetMaterial(0, Material);
		}
	}
}


void ACubiquityVolume::commitChanges()
{
	if (volume())
	{
		volume()->acceptOverrideChunks();
	}
}

void ACubiquityVolume::discardChanges()
{
	if (volume())
	{
		volume()->discardOverrideChunks();
	}
}

FVector ACubiquityVolume::worldPositionToVolumePosition(const FVector& worldPosition) const
{
	return ActorToWorld().InverseTransformPosition(worldPosition);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("World: %f %f %f,  Mesh: %f %f %f,  Voxel: %d %d %d"), worldPosition.X, worldPosition.Y, worldPosition.Z, meshSpacePosition.X, meshSpacePosition.Y, meshSpacePosition.Z, int32_t(meshSpacePosition.X), int32_t(meshSpacePosition.Y), int32_t(meshSpacePosition.Z)));
	//return{ int32_t(meshSpacePosition.X), int32_t(meshSpacePosition.Y), int32_t(meshSpacePosition.Z) };
}

FVector ACubiquityVolume::volumePositionToWorldPosition(const FVector& localPosition) const
{
	return ActorToWorld().TransformPosition(localPosition);
}

FVector ACubiquityVolume::worldDirectionToVolumeDirection(const FVector& worldDirection) const
{
	return ActorToWorld().InverseTransformVectorNoScale(worldDirection);
}

FVector ACubiquityVolume::volumeDirectionToWorldDirection(const FVector& localDirection) const
{
	return ActorToWorld().TransformVectorNoScale(localDirection);
}

FVector ACubiquityVolume::eyePositionInVolumeSpace() const
{
	UWorld* const World = GetWorld();
	if (World)
	{
		//UE_LOG(CubiquityLog, Log, TEXT("Found world"));
		auto playerController = World->GetFirstPlayerController();
		if (playerController)
		{
			//UE_LOG(CubiquityLog, Log, TEXT("Found PC"));
			FVector location;
			FRotator rotation;
			playerController->GetPlayerViewPoint(location, rotation);
			//UE_LOG(CubiquityLog, Log, TEXT("Location: %f %f %f"), location.X, location.Y, location.Z);
			//UE_LOG(CubiquityLog, Log, TEXT("Location: %f %f %f"), worldToVolume(location).X, worldToVolume(location).Y, worldToVolume(location).Z);
			return worldPositionToVolumePosition(location);
		}
	}

	return {0.0, 0.0, 0.0};
	
}
