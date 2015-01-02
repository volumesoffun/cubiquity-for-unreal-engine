// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "CubiquityVolume.h"

#include "CubiquityColoredCubesVolume.generated.h"

class UCubiquityMeshComponent;

/**
*
*/
UCLASS()
class ACubiquityColoredCubesVolume : public ACubiquityVolume
{
	GENERATED_BODY()

public:

	ACubiquityColoredCubesVolume(const FObjectInitializer& PCIP);
	virtual void PostActorCreated() override;
	virtual void PostLoad() override;

	virtual void Destroyed() override;

	//Along a raycast, get the position of the first non-empty voxel
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FVector pickFirstSolidVoxel(FVector localStartPosition, FVector localDirection);

	//Along a raycast, get the position of the last empty voxel
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FVector pickLastEmptyVoxel(FVector localStartPosition, FVector localDirection);

	//Set a voxel in the volume to a specific value
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void setVoxel(FIntVector localPosition, FColor newColor);

	//Get the value of a voxel in the terrain
	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FColor getVoxel(FIntVector localPosition);

private:
	std::unique_ptr<Cubiquity::ColoredCubesVolume> m_volume = nullptr;
	Cubiquity::Volume* volume() override { return m_volume.get(); }

	void loadVolume() override;
};
