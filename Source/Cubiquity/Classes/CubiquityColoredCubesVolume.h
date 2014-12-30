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
	FVector pickFirstSolidVoxel(FVector localStartPosition, FVector localDirection);

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FVector pickLastEmptyVoxel(FVector localStartPosition, FVector localDirection);

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void setVoxel(FIntVector localPosition, FColor newColor);

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FColor getVoxel(FIntVector localPosition);

private:
	std::unique_ptr<Cubiquity::ColoredCubesVolume> m_volume = nullptr;
	Cubiquity::Volume* volume() override { return m_volume.get(); }

	void loadVolume() override;
};
