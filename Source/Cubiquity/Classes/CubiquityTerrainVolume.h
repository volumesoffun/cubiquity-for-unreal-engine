// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "CubiquityVolume.h"

#include "CubiquityTerrainVolume.generated.h"

class UCubiquityMeshComponent;
class UCubiquityMaterialSet;

/**
* A voxel terrain object that uses marching cubes
*/
UCLASS()
class ACubiquityTerrainVolume : public ACubiquityVolume
{
	GENERATED_BODY()

public:

	ACubiquityTerrainVolume(const FObjectInitializer& PCIP);
	virtual void PostActorCreated() override;
	virtual void PostLoad() override;

	virtual void Destroyed() override;

	/**
	* \param localPosition the volume-space position of the position to sculpt
	* \param innerRadius the volume-space size of the solid part of the brush
	* \param outerRadius the volume-space radius of the fall-off region of the brush
	* \param opacity
	*/
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void sculptTerrain(FVector localPosition, float innerRadius = 0.5, float outerRadius = 2.0, float opacity = 0.8);

	/**
	 * \param localStartPosition the volume-space position of the start of the raycast
	 * \param localDirection the volume-space direction of the raycast
	 * \return the volume-space position of the raycast hit
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cubiquity")
	FVector pickSurface(FVector localStartPosition, FVector localDirection) const;

	//Set a voxel in the volume to a specific value
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void setVoxel(FVector localPosition, const UCubiquityMaterialSet* materialSet);

	//Get the value of a voxel in the terrain
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cubiquity")
	UCubiquityMaterialSet* getVoxel(FVector localPosition) const;

private:
	std::unique_ptr<Cubiquity::TerrainVolume> m_volume = nullptr;
	Cubiquity::Volume* volume() override { return m_volume.get(); }

	void loadVolume() override;
};
