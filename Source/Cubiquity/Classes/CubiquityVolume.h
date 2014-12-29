// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Cubiquity.hpp"

#include "CubiquityOctreeNode.h"

#include "GameFramework/Actor.h"

#include <memory>

#include "CubiquityVolume.generated.h"

class UCubiquityMeshComponent;
class UCubiquityUpdateComponent;

/**
* A CubiquityVolume is the base class for the volume actors in Cubiquity.
* It is an abstact class with derived classes for the types of terrain supported.
* A CubiquityVolume has no visual representation in the world but instead holds an ACubiquityOctreeNode tree which have visual components.
*/
UCLASS(Abstract)
class ACubiquityVolume : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void PostActorCreated() override;
	virtual void PostLoad() override;
	virtual void OnConstruction(const FTransform & transform) override;
	virtual void PostInitializeComponents();
	virtual void BeginPlay() override;
	//PostEditChangeProperty for editing in the editor

	virtual void Destroyed() override;

	virtual void Tick(float DeltaSeconds) override;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cubiquity")
	UMaterialInterface* Material; //The material applied to the terrain

	UCubiquityUpdateComponent* root; //Used to attach the mesh component hierachy to

	UPROPERTY(EditAnywhere, Category = "Cubiquity")
	FString volumeFileName;

	UPROPERTY(EditAnywhere, Category = "Cubiquity")
	float lodThreshold = 1.0;

	//This should be called after setting the material to propgate the change
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void updateMaterial();

	//This saves the temporary changes made to the volume in memory to the backing store
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	virtual void commitChanges() const PURE_VIRTUAL(ACubiquityVolume::commitChanges, );

	//This discards the temporary changes made to the volume
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	virtual void discardChanges() const PURE_VIRTUAL(ACubiquityVolume::discardChanges, );

protected:

	FVector eyePositionInVolumeSpace() const;

	ACubiquityOctreeNode* octreeRootNodeActor = nullptr;

	virtual void loadVolume() PURE_VIRTUAL(ACubiquityVolume::loadVolume,);

	FVector worldToVolume(FVector worldPosition) const;
	FVector volumeToWorld(FVector localPosition) const;

};
