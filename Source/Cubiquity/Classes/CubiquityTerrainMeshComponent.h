// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "Cubiquity.hpp"

#include "CubiquityMeshComponent.h"

#include <cstdint>

#include <DynamicMeshBuilder.h>

#include "CubiquityTerrainMeshComponent.generated.h"

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class UCubiquityTerrainMeshComponent : public UCubiquityMeshComponent
{
	GENERATED_UCLASS_BODY()
};
