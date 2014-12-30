// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityVolume.h"
#include "CubiquityOctreeNode.h"
#include "CubiquityTerrainMeshComponent.h"
#include "CubiquityUpdateComponent.h"

ACubiquityVolume::ACubiquityVolume(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	UE_LOG(CubiquityLog, Log, TEXT("Creating ACubiquityVolume"));
	root = PCIP.CreateDefaultSubobject<UCubiquityUpdateComponent>(this, TEXT("Root"));
	RootComponent = root;

	PrimaryActorTick.bCanEverTick = true;
}

void ACubiquityVolume::PostActorCreated()
{
	Super::PostActorCreated();
}

void ACubiquityVolume::PostLoad()
{
	Super::PostLoad();
}

void ACubiquityVolume::OnConstruction(const FTransform& transform)
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityVolume::OnConstruction"));

	if (RootComponent->GetNumChildrenComponents() == 0) //If we haven't created the octree yet
	{
		createOctree();
	}

	Super::OnConstruction(transform);
}

void ACubiquityVolume::PostInitializeComponents()
{
	UE_LOG(CubiquityLog, Log, TEXT("ACubiquityVolume::PostInitializeComponents"));

	//generateMeshes();

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

	/*TArray<AActor*> children = Children; //Make a copy to avoid overruns
	//UE_LOG(CubiquityLog, Log, TEXT(" Children %d"), children.Num());
	for (AActor* childActor : children) //Should only be 1 child of this Actor
	{
		//UE_LOG(CubiquityLog, Log, TEXT("  Destroying childActor"));
		GetWorld()->DestroyActor(childActor);
	}*/

	Super::Destroyed();
}

void ACubiquityVolume::Tick(float DeltaSeconds)
{
	//UE_LOG(CubiquityLog, Log, TEXT("ACubiquityVolume::Tick"));

	Super::Tick(DeltaSeconds);
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

FVector ACubiquityVolume::worldToVolume(FVector worldPosition) const
{
	return root->GetComponentTransform().InverseTransformPosition(worldPosition);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("World: %f %f %f,  Mesh: %f %f %f,  Voxel: %d %d %d"), worldPosition.X, worldPosition.Y, worldPosition.Z, meshSpacePosition.X, meshSpacePosition.Y, meshSpacePosition.Z, int32_t(meshSpacePosition.X), int32_t(meshSpacePosition.Y), int32_t(meshSpacePosition.Z)));
	//return{ int32_t(meshSpacePosition.X), int32_t(meshSpacePosition.Y), int32_t(meshSpacePosition.Z) };
}

FVector ACubiquityVolume::volumeToWorld(FVector localPosition) const
{
	return root->GetComponentTransform().TransformPosition(localPosition);
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
			return worldToVolume(location);
		}
	}

	return {0.0, 0.0, 0.0};
	
}
