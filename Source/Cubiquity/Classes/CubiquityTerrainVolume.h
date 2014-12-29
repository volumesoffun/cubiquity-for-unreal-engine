// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "CubiquityVolume.h"

#include "CubiquityMaterialSet.h"

#include "CubiquityTerrainVolume.generated.h"

class UCubiquityMeshComponent;

/**
*
*/
UCLASS()
class ACubiquityTerrainVolume : public ACubiquityVolume
{
	GENERATED_UCLASS_BODY()

	virtual void PostActorCreated() override;
	virtual void PostLoad() override;

	virtual void Destroyed() override;

	void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void sculptTerrain(const FVector& worldPosition, float innerRadius = 0.5, float outerRadius = 2.0, float opacity = 0.8); //TODO in world space

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	FVector pickSurface(const FVector& start, const FVector& direction); //TODO returns in world space

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void setVoxel(const FIntVector& position, const UCubiquityMaterialSet* materialSet); //In volume space

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	UCubiquityMaterialSet* getVoxel(const FIntVector& position); //In volume space

	void commitChanges() const override { if (volume){ volume->acceptOverrideChunks(); } }
	void discardChanges() const override { if (volume){ volume->discardOverrideChunks(); } }

private:
	std::unique_ptr<Cubiquity::TerrainVolume> volume = nullptr;

	virtual void loadVolume() override;
};
