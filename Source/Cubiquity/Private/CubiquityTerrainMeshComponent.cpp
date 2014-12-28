// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved. 

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityTerrainMeshComponent.h"

UCubiquityTerrainMeshComponent::UCubiquityTerrainMeshComponent(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = false;
}
