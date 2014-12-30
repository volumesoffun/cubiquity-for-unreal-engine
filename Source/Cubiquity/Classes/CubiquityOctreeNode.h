// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include <memory>

#include "Cubiquity.hpp"

#include "CubiquityOctreeNode.generated.h"

class ACubiquityVolume;
class UCubiquityMeshComponent;

/**
 * This is marked transient so that Cubiquity can recreate on level loading
 * These objects can be created and destroyed by Cubiquity as the structure of the octree changes.
 */
UCLASS(Transient)
class ACubiquityOctreeNode : public AActor
{
	GENERATED_BODY()

public:

	ACubiquityOctreeNode(const FObjectInitializer& PCIP);

	virtual void Destroyed() override;

	UCubiquityMeshComponent* mesh = nullptr;

	void initialiseOctreeNode(const Cubiquity::OctreeNode& newOctreeNode, USceneComponent* parent, UMaterialInterface* material);

	void processOctreeNode(const Cubiquity::OctreeNode& octreeNode);

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	ACubiquityVolume* getVolume() const;

	Cubiquity::Vector<int32_t> octreeNodePosition() const { return m_octreeNodePosition; }

private:

	ACubiquityOctreeNode* children[2][2][2];

	//This could maybe be given by actor position passed through worldToVolume?
	Cubiquity::Vector<int32_t> m_octreeNodePosition;

	uint32_t structureLastSynced = 0;
	uint32_t propertiesLastSynced = 0;
	uint32_t meshLastSynced = 0;
	uint32_t nodeAndChildrenLastSynced = 0;
	uint8_t height = 0;
	bool renderThisNode = false;
	
};
