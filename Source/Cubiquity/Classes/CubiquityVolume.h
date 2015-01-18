// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "Cubiquity.hpp"

#include <memory>

#include "CubiquityVolume.generated.h"

class UCubiquityMeshComponent;
class UCubiquityUpdateComponent;
class ACubiquityOctreeNode;

/**
* A CubiquityVolume is the base class for the volume actors in Cubiquity.
* It is an abstact class with derived classes for the types of terrain supported.
* A CubiquityVolume has no visual representation in the world but instead holds an ACubiquityOctreeNode tree which have visual components.
*/
UCLASS(Abstract)
class ACubiquityVolume : public AActor
{
	GENERATED_BODY()

public:

	ACubiquityVolume(const FObjectInitializer& PCIP);
	virtual void PostActorCreated() override;
	virtual void PostLoad() override;
	virtual void OnConstruction(const FTransform & transform) override;
	virtual void PostInitializeComponents() override;
	//virtual void PostActorConstruction() override;
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	void processOctree();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;
#endif

	/** The material applied to the terrain */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cubiquity")
	UMaterialInterface* Material;

	/** The filename on disk to load from or save to */
	UPROPERTY(EditAnywhere, Category = "Cubiquity")
	FString volumeFileName;

	/** How aggressive the LOD scaling is. Low numbers are better quality. Usually between 0.5 and 2.0 */
	UPROPERTY(EditAnywhere, Category = "Cubiquity")
	float lodThreshold = 1.0;

	//This should be called after setting the material to propgate the change
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void updateMaterial();

	//This saves the temporary changes made to the volume in memory to the backing store
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void commitChanges();

	//This discards the temporary changes made to the volume
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void discardChanges();

	// Convert fom world-space to volume-space
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cubiquity")
	FVector worldPositionToVolumePosition(const FVector& worldPosition) const;

	// Convert fom volume-space to world-space
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cubiquity")
	FVector volumePositionToWorldPosition(const FVector& localPosition) const;

	// Convert fom world-space to volume-space
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cubiquity")
	FVector worldDirectionToVolumeDirection(const FVector& worldDirection) const;

	// Convert fom volume-space to world-space
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cubiquity")
	FVector volumeDirectionToWorldDirection(const FVector& localDirection) const;

protected:

	//Used to attach the mesh component hierachy to
	UCubiquityUpdateComponent* root;

	//This provides access to the subclass' volume pointer in a subtype-independant way
	//We can access all the general Volume stuff by this
	//It returns a non-owning pointer which should not be stored and only used directly
	virtual Cubiquity::Volume* volume() PURE_VIRTUAL(ACubiquityVolume::getVolume, return nullptr;);

	//The the rendering position for the volume mesh extraction
	FVector eyePositionInVolumeSpace() const;

	//This is the root of the octree for our volume
	ACubiquityOctreeNode* octreeRootNodeActor = nullptr;

	//Create the octreeRootNodeActor and propagate down the tree
	void createOctree();

	//Load the volume into memory based on volumeFileName
	//The subclasses implementation of this will call loadVolumeImpl() with the correct template type
	virtual void loadVolume() PURE_VIRTUAL(ACubiquityVolume::loadVolume, );

	//This function is here and templated to avoid code duplication due to different volume types
	template <typename VolumeType>
	std::unique_ptr<VolumeType> loadVolumeImpl()
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*volumeFileName))
		{
			return std::make_unique<VolumeType>(TCHAR_TO_ANSI(*volumeFileName), Cubiquity::WritePermissions::ReadOnly, 32);
		}
		else
		{
			return std::make_unique<VolumeType>(Cubiquity::Vector<int32_t>{ 0, 0, 0 }, Cubiquity::Vector<int32_t>{ 128, 128, 32 }, TCHAR_TO_ANSI(*volumeFileName), 32);
		}
	}

};
