// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

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

	void commitChanges() const override { if (volume){ volume->acceptOverrideChunks(); } }
	void discardChanges() const override { if (volume){ volume->discardOverrideChunks(); } }

private:
	std::unique_ptr<Cubiquity::ColoredCubesVolume> volume = nullptr;

	virtual void loadVolume() override;

};
