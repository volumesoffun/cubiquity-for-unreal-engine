// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityUpdateComponent.h"

#include "CubiquityVolume.h"

UCubiquityUpdateComponent::UCubiquityUpdateComponent(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
}

void UCubiquityUpdateComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetAttachmentRootActor())
	{
		Cast<ACubiquityVolume>(GetAttachmentRootActor())->processOctree();
	}
};
