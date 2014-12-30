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
	GENERATED_UCLASS_BODY()

	virtual void PostActorCreated() override;
	virtual void PostLoad() override;

	virtual void Destroyed() override;

	void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FVector pickFirstSolidVoxel(FVector start, FVector direction); //Currently in world space

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FVector pickLastEmptyVoxel(FVector start, FVector direction); //Currently in world space

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void setVoxel(FIntVector position, FColor newColor); //In volume space

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FColor getVoxel(FIntVector position); //In volume space

private:
	std::unique_ptr<Cubiquity::ColoredCubesVolume> m_volume = nullptr;
	Cubiquity::Volume* volume() override { return m_volume.get(); }

	void loadVolume() override;
};
