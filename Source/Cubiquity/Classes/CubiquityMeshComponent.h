// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "Cubiquity.hpp"

#include <cstdint>

#include <DynamicMeshBuilder.h>

#include "CubiquityColoredCubesVertexFactory.h"
#include "CubiquityTerrainVertexFactory.h"

#include "CubiquityMeshComponent.generated.h"

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class UCubiquityMeshComponent : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()

public:

	UCubiquityMeshComponent(const FObjectInitializer& PCIP);

	/** Set the geometry to use on this triangle mesh */
	bool SetGeneratedMeshTriangles(const Cubiquity::OctreeNode& octreeNode);

	bool SetGeneratedMeshTrianglesColoredCubes(const Cubiquity::OctreeNode& octreeNode);
	bool SetGeneratedMeshTrianglesTerrain(const Cubiquity::OctreeNode& octreeNode);

	UFUNCTION(BlueprintCallable, Category = "Components|GeneratedMesh")
	bool ClearMeshTriangles();

	/** Description of collision */
	UPROPERTY(BlueprintReadOnly, Category = "Collision")
	class UBodySetup* ModelBodySetup;

	// Begin UMeshComponent interface.
	virtual int32 GetNumMaterials() const override;
	// End UMeshComponent interface.

	// Begin Interface_CollisionDataProvider Interface
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh() override { return false; }
	// End Interface_CollisionDataProvider Interface

	// Begin UPrimitiveComponent interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual class UBodySetup* GetBodySetup() override;
	// End UPrimitiveComponent interface.

	void UpdateBodySetup();
	void UpdateCollision();

	void setVolumeType();

private:

	// Begin USceneComponent interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;
	// Begin USceneComponent interface.

	/** */
	TArray<FDynamicMeshVertex> terrainVertices;
	TArray<FColoredCubesVertex> coloredCubesVertices; //TODO It's horrible that we have a different vertex list for the different terrain types. This is due to differing vertex types and data layout.
	TArray<int32> indices;

	Cubiquity::VolumeType volumeType;

	friend class FGeneratedMeshSceneProxy;
	friend class FColoredCubesSceneProxy;
};
