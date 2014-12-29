// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityTerrainMeshComponent.h"

UCubiquityTerrainMeshComponent::UCubiquityTerrainMeshComponent(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = false;
}
