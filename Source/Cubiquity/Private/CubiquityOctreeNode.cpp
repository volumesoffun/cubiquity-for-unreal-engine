// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityOctreeNode.h"
#include "CubiquityVolume.h"
#include "CubiquityMeshComponent.h"

ACubiquityOctreeNode::ACubiquityOctreeNode(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	//Create mesh component
	//UE_LOG(CubiquityLog, Log, TEXT("ACubiquityOctreeNode constructor"));
	mesh = PCIP.CreateDefaultSubobject<UCubiquityMeshComponent>(this, TEXT("mesh"));
	RootComponent = mesh;

	for (uint32_t z = 0; z < 2; z++)
	{
		for (uint32_t y = 0; y < 2; y++)
		{
			for (uint32_t x = 0; x < 2; x++)
			{
				children[x][y][z] = nullptr;
			}
		}
	}
}

void ACubiquityOctreeNode::Destroyed()
{
	//TODO Replace traversal with reading 'children' array member?

	//UE_LOG(CubiquityLog, Log, TEXT("ACubiquityOctreeNode::Destroyed"));

	TArray<AActor*> childrenActors = Children;
	//UE_LOG(CubiquityLog, Log, TEXT(" Children %d"), childrenActors.Num());
	for (AActor* childActor : childrenActors)
	{
		//UE_LOG(CubiquityLog, Log, TEXT("  Destroying childActor"));
		GetWorld()->DestroyActor(childActor);
	}

	Super::Destroyed();
}

void ACubiquityOctreeNode::initialiseOctreeNode(const Cubiquity::OctreeNode& newOctreeNode, USceneComponent* parentComponent, UMaterialInterface* material)
{
	if (parentComponent)
	{
		mesh->AttachTo(parentComponent);
	}

	//UE_LOG(CubiquityLog, Log, TEXT("%d My absolute: %d %d %d     Parent absolute: %d %d %d     Relative: %d %d %d"), depth, nodeX, nodeY, nodeZ, parentX, parentY, parentZ, nodeX - parentX, nodeY - parentY, nodeZ - parentZ);
	
	structureLastSynced = 0;
	propertiesLastSynced = 0;
	meshLastSynced = 0;
	nodeAndChildrenLastSynced = 0;

	m_octreeNodePosition = newOctreeNode.position();

	mesh->setVolumeType();

	mesh->SetMaterial(0, material);
}

void ACubiquityOctreeNode::processOctreeNode(const Cubiquity::OctreeNode& octreeNode)
{
	if (octreeNode.nodeOrChildrenLastChanged() > nodeAndChildrenLastSynced)
	{
		if (octreeNode.propertiesLastChanged() > propertiesLastSynced)
		{
			height = octreeNode.height();
			renderThisNode = octreeNode.renderThisNode();

			mesh->SetVisibility(renderThisNode); //Hide the mesh as needed

			propertiesLastSynced = Cubiquity::currentTime();
		}

		if (octreeNode.meshLastChanged() > meshLastSynced)
		{
			//Set the position of our component relative to the position of the mesh's parent
			//TODO we shouldn't try to move it here since it should be static.
			Cubiquity::Vector<int32_t> parentPosition;
			ACubiquityOctreeNode* parentOctreeNodeActor = Cast<ACubiquityOctreeNode>(GetOwner());
			if (parentOctreeNodeActor)
			{
				parentPosition = parentOctreeNodeActor->octreeNodePosition();
			}
			else
			{
				parentPosition = { 0, 0, 0 };
			}
			const auto position = octreeNode.position();
			mesh->SetRelativeLocation(FVector(position.x - parentPosition.x, position.y - parentPosition.y, position.z - parentPosition.z));
			
			if (octreeNode.hasMesh())
			{
				//Reset all the mesh data and repopulate it
				mesh->ClearMeshTriangles();
				mesh->SetGeneratedMeshTriangles(octreeNode);
			}
			else
			{
				mesh->ClearMeshTriangles();
			}

			meshLastSynced = Cubiquity::currentTime();
		}

		if (octreeNode.structureLastChanged() > structureLastSynced)
		{
			for (uint32_t z = 0; z < 2; z++)
			{
				for (uint32_t y = 0; y < 2; y++)
				{
					for (uint32_t x = 0; x < 2; x++)
					{
						ACubiquityOctreeNode* const childActor = children[x][y][z];

						if (octreeNode.hasChildNode({ x, y, z }))
						{
							if (!childActor) //If we don't have a child actor but there is a child node ... create it
							{
								const auto& childNode = octreeNode.childNode({ x, y, z });

								FActorSpawnParameters spawnParameters;
								spawnParameters.Owner = this;
								ACubiquityOctreeNode* childNodeActor = GetWorld()->SpawnActor<ACubiquityOctreeNode>(spawnParameters);
								childNodeActor->initialiseOctreeNode(childNode, mesh, getVolume()->Material);

								children[x][y][z] = childNodeActor;
							}
						}
						else
						{
							if (childActor) //If we have a child actor but there is no child node ... delete it
							{
								children[x][y][z]->Destroy();
								children[x][y][z] = nullptr;
							}
						}
					}
				}
			}

			structureLastSynced = Cubiquity::currentTime();
		}

		for (uint32_t z = 0; z < 2; z++)
		{
			for (uint32_t y = 0; y < 2; y++)
			{
				for (uint32_t x = 0; x < 2; x++)
				{
					if (octreeNode.hasChildNode({ x, y, z }))
					{
						// Recursivly call the octree traversal
						children[x][y][z]->processOctreeNode(octreeNode.childNode({ x, y, z }));
					}
				}
			}
		}

		nodeAndChildrenLastSynced = Cubiquity::currentTime();
	}
}

ACubiquityVolume* ACubiquityOctreeNode::getVolume() const
{
	return Cast<ACubiquityVolume>(mesh->GetAttachmentRootActor());
}
