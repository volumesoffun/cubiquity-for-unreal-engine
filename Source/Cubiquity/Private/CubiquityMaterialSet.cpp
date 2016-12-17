// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityMaterialSet.h"

UCubiquityMaterialSet::UCubiquityMaterialSet() : Super()
{
}

void UCubiquityMaterialSet::setMaterial(uint8 index, uint8 value)
{
	m_materialSet.setMaterial(index, value);
}

uint8 UCubiquityMaterialSet::getMaterial(uint8 index) const
{
	return m_materialSet.getMaterial(index);
}

UCubiquityMaterialSet* UCubiquityMaterialSet::MakeCubiquityMaterialSet()
{
	return NewObject<UCubiquityMaterialSet>();
}
