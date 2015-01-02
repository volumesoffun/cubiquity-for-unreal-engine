// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "CubiquityUpdateComponent.generated.h"

/** 
 * The whole purpose of this class is to allow the volume actor to process updates while in the editor
 * Its TickComponent function simply calls its owner's Tick function
 */
UCLASS()
class UCubiquityUpdateComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UCubiquityUpdateComponent(const FObjectInitializer& PCIP);

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
