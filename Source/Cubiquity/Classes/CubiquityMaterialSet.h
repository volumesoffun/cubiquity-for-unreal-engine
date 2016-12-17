// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "Cubiquity.hpp"

#include "CubiquityMaterialSet.generated.h"

UCLASS(BlueprintType)
class UCubiquityMaterialSet : public UObject
{
	GENERATED_BODY()

public:
	UCubiquityMaterialSet();

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	void setMaterial(uint8 index, uint8 value);

	UFUNCTION(BlueprintCallable, Category = "Cubiquity")
	uint8 getMaterial(uint8 index) const;

	UFUNCTION(BlueprintPure, Category = "Cubiquity")
	static UCubiquityMaterialSet* MakeCubiquityMaterialSet();

	operator Cubiquity::MaterialSet() const { return m_materialSet; }

private:
	friend class ACubiquityTerrainVolume;
	Cubiquity::MaterialSet m_materialSet;
};
