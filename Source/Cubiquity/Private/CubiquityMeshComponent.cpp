// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityMeshComponent.h"
#include "CubiquityTerrainVolume.h"
#include "CubiquityColoredCubesVolume.h"

#include "PhysicsEngine/BodySetup.h"

UCubiquityMeshComponent::UCubiquityMeshComponent(const FObjectInitializer& PCIP)
	: Super(PCIP)
{
	//UE_LOG(CubiquityLog, Log, TEXT("Creating UCubiquityMeshComponent"));
	PrimaryComponentTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Stationary);
	SetCollisionObjectType(ECC_WorldStatic);
}

void UCubiquityMeshComponent::setVolumeType()
{
	if (Cast<ACubiquityTerrainVolume>(GetAttachmentRootActor()))
	{
		volumeType = Cubiquity::VolumeType::Terrain;
	}
	else if (Cast<ACubiquityColoredCubesVolume>(GetAttachmentRootActor()))
	{
		volumeType = Cubiquity::VolumeType::ColoredCubes;
	}
	else
	{
		UE_LOG(CubiquityLog, Warning, TEXT("setVolumeType failed to determine the type of the volume"));
	}
}

bool UCubiquityMeshComponent::SetGeneratedMeshTriangles(const Cubiquity::OctreeNode& octreeNode)
{
	switch (volumeType)
	{
		case Cubiquity::VolumeType::Terrain:
			return SetGeneratedMeshTrianglesTerrain(octreeNode);
		case Cubiquity::VolumeType::ColoredCubes:
			return SetGeneratedMeshTrianglesColoredCubes(octreeNode);
		default:
			return false;
	}
}

bool UCubiquityMeshComponent::SetGeneratedMeshTrianglesTerrain(const Cubiquity::OctreeNode& octreeNode)
{
	//UE_LOG(CubiquityLog, Log, TEXT("UCubiquityMeshComponent::SetGeneratedMeshTrianglesTerrain"));

	uint32_t noOfIndices;
	uint16_t* cubuquityIndices;
	uint16_t noOfVertices;
	const Cubiquity::TerrainVertex* cubiquityVertices;
	octreeNode.getMesh(&noOfVertices, &cubiquityVertices, &noOfIndices, &cubuquityIndices);

	for (uint32_t i = 0; i < noOfVertices; ++i)
	{
		const auto& cubiquityVertex = cubiquityVertices[i];

		FDynamicMeshVertex Vert;

		const auto position = cubiquityVertex.position();
		Vert.Position = FVector(position.x, position.y, position.z);

		const auto normal = cubiquityVertex.normal();
		Vert.TangentZ = FVector(normal.x, normal.y, normal.z);// .SafeNormal();

		const auto materials = cubiquityVertex.materials();
		Vert.Color = FColor(materials[0], materials[1], materials[2], materials[3]); //TODO make this from materials

		Vert.TextureCoordinate.Set(Vert.Position.X, Vert.Position.Y);

		terrainVertices.Add(Vert);
	}

	//The normals are already set but we need the tangents and bitangents. Set these on a per-triangle basis based on the geometry
	for (uint16_t i = 0; i < noOfIndices; ++i)
	{
		const uint16_t index0 = cubuquityIndices[i];
		const uint16_t index1 = cubuquityIndices[++i];
		const uint16_t index2 = cubuquityIndices[++i];

		FDynamicMeshVertex& vertex0 = terrainVertices[index0];
		FDynamicMeshVertex& vertex1 = terrainVertices[index1];
		FDynamicMeshVertex& vertex2 = terrainVertices[index2];

		//Reverse winding order
		indices.Add(index2);
		indices.Add(index1);
		indices.Add(index0);

		//Now calculate the tangent vectors
		const FVector Edge01 = (vertex1.Position - vertex0.Position);
		const FVector Edge02 = (vertex2.Position - vertex0.Position);
		const FVector TangentX = Edge01.GetSafeNormal() * 256.0; //Tangent
		const FVector TangentZ = vertex0.TangentZ; //Normal
		const FVector TangentY = (TangentX ^ TangentZ); //Binormal (bitangent) I assume?

		vertex1.SetTangents(TangentX, TangentY, vertex1.TangentZ);
		vertex2.SetTangents(TangentX, TangentY, vertex2.TangentZ);
	}

	if (ModelBodySetup)
	{
		ModelBodySetup->InvalidatePhysicsData(); // This is required for the first time after creation
	}

	UpdateCollision();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();

	return true;
}

bool UCubiquityMeshComponent::SetGeneratedMeshTrianglesColoredCubes(const Cubiquity::OctreeNode& octreeNode)
{
	//UE_LOG(CubiquityLog, Log, TEXT("UCubiquityMeshComponent::SetGeneratedMeshTrianglesColoredCubes"));

	uint32_t noOfIndices;
	uint16_t* cubuquityIndices;
	uint16_t noOfVertices;
	Cubiquity::ColoredCubesVertex* cubiquityVertices;
	octreeNode.getMesh(&noOfVertices, &cubiquityVertices, &noOfIndices, &cubuquityIndices);

	for (uint32_t i = 0; i < noOfVertices; ++i)
	{
		const auto& cubiquityVertex = cubiquityVertices[i];

		FColoredCubesVertex Vert;

		const auto& position = cubiquityVertex.position();
		Vert.Position = FVector(position.x, position.y, position.z);

		const auto& color = cubiquityVertex.color();
		Vert.Color = FColor(color.red(), color.green(), color.blue(), color.alpha());

		coloredCubesVertices.Add(Vert);
	}

	//TODO: Could we do these 6 at at time for each quad?
	for (uint16_t i = 0; i < noOfIndices; ++i)
	{
		const uint16_t index0 = cubuquityIndices[i];
		const uint16_t index1 = cubuquityIndices[++i];
		const uint16_t index2 = cubuquityIndices[++i];

		const FColoredCubesVertex& vertex0 = coloredCubesVertices[index0];
		const FColoredCubesVertex& vertex1 = coloredCubesVertices[index1];
		const FColoredCubesVertex& vertex2 = coloredCubesVertices[index2];

		//Reverse winding order
		indices.Add(index2);
		indices.Add(index1);
		indices.Add(index0);

		//Now calculate the tangent vectors
		const FVector Edge01 = (vertex1.Position - vertex0.Position);
		const FVector Edge02 = (vertex2.Position - vertex0.Position);
		const FVector normal = (Edge02 ^ Edge01); //Normal

		/*vertex0.SetTangents(TangentX, TangentY, TangentZ);
		vertex1.SetTangents(TangentX, TangentY, TangentZ);
		vertex2.SetTangents(TangentX, TangentY, TangentZ);*/
	}

	if (ModelBodySetup)
	{
		ModelBodySetup->InvalidatePhysicsData(); // This is required for the first time after creation
	}

	UpdateCollision();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();

	return true;
}

bool UCubiquityMeshComponent::ClearMeshTriangles()
{
	//UE_LOG(CubiquityLog, Log, TEXT("UCubiquityMeshComponent::ClearMeshTriangles"));
	terrainVertices.Empty();
	coloredCubesVertices.Empty();
	indices.Empty();

	if (ModelBodySetup)
	{
		ModelBodySetup->InvalidatePhysicsData(); // This is required for the first time after creation
	}

	UpdateCollision();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();

	return true;
}

FPrimitiveSceneProxy* UCubiquityMeshComponent::CreateSceneProxy()
{
	//UE_LOG(CubiquityLog, Log, TEXT("UCubiquityMeshComponent::CreateSceneProxy"));
	FPrimitiveSceneProxy* Proxy = nullptr;

	if (terrainVertices.Num() > 0 || coloredCubesVertices.Num() > 0)
	{
		if (volumeType == Cubiquity::VolumeType::Terrain)
		{
			Proxy = new FGeneratedMeshSceneProxy(this);
		}
		else if (volumeType == Cubiquity::VolumeType::ColoredCubes)
		{
			Proxy = new FColoredCubesSceneProxy(this);
		}
		else
		{
			UE_LOG(CubiquityLog, Warning, TEXT("2 OTHER!"));
		}
	}
	return Proxy;
}

int32 UCubiquityMeshComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds UCubiquityMeshComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	//UE_LOG(CubiquityLog, Log, TEXT("UCubiquityMeshComponent::CalcBounds"));
	/*FBoxSphereBounds NewBounds;
	NewBounds.Origin = FVector::ZeroVector;
	NewBounds.BoxExtent = FVector(HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	NewBounds.SphereRadius = FMath::Sqrt(3.0f * FMath::Square(HALF_WORLD_MAX));
	return NewBounds;*/
	//FVector BoxPoint = FVector(100, 100, 100);
	/*const FVector UnitV(1, 1, 1);
	const FBox VeryVeryBig(-UnitV * HALF_WORLD_MAX, UnitV * HALF_WORLD_MAX);
	return FBoxSphereBounds(VeryVeryBig);*/
	return FBoxSphereBounds(FBox(FVector(-128, -128, -128), FVector(128, 128, 128))).TransformBy(LocalToWorld); //TODO check this for each node...
}


bool UCubiquityMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	if (ContainsPhysicsTriMeshData(true))
	{
		if (volumeType == Cubiquity::VolumeType::Terrain)
		{
			for (const auto& vertex : terrainVertices)
			{
				CollisionData->Vertices.Add(vertex.Position);
			}
		}
		else if (volumeType == Cubiquity::VolumeType::ColoredCubes)
		{
			for (const auto& vertex : coloredCubesVertices)
			{	
				CollisionData->Vertices.Add(vertex.Position);
			}
		}
		
		for (auto index = indices.CreateConstIterator(); index;)
		{
			FTriIndices Triangle;

			Triangle.v0 = *index++;
			Triangle.v1 = *index++;
			Triangle.v2 = *index++;

			CollisionData->Indices.Add(Triangle);
			//CollisionData->MaterialIndices.Add(i); //For physical material properties
		}

		CollisionData->bFlipNormals = true;

		return true;
	}

	return false;
}

bool UCubiquityMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return (terrainVertices.Num() > 0 || coloredCubesVertices.Num() > 0) && (indices.Num() > 0);
}

void UCubiquityMeshComponent::UpdateBodySetup()
{
	//UE_LOG(CubiquityLog, Log, TEXT("UCubiquityMeshComponent::UpdateBodySetup"));
	if (!ModelBodySetup)
	{
		SetSimulatePhysics(false);
		ModelBodySetup = NewObject<UBodySetup>(this);
		ModelBodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
		ModelBodySetup->bMeshCollideAll = true;
	}
}

void UCubiquityMeshComponent::UpdateCollision()
{
	//UE_LOG(CubiquityLog, Log, TEXT("UCubiquityMeshComponent::UpdateCollision"));
	if (bPhysicsStateCreated)
	{
		DestroyPhysicsState();
		UpdateBodySetup();
		CreatePhysicsState();

		ModelBodySetup->InvalidatePhysicsData();
		ModelBodySetup->CreatePhysicsMeshes();

		//UE_LOG(CubiquityLog, Log, TEXT("Physics updated"));
	}
}

UBodySetup* UCubiquityMeshComponent::GetBodySetup()
{
	UpdateBodySetup();
	return ModelBodySetup;
}
