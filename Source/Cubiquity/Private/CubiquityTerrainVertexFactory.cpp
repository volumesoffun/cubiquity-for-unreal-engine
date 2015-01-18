// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityTerrainVertexFactory.h"

#include "CubiquityMeshComponent.h"

void FGeneratedMeshVertexFactory::Init(const FGeneratedMeshVertexBuffer* VertexBuffer)
{
	check(!IsInRenderingThread());

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		InitGeneratedMeshVertexFactory,
		FGeneratedMeshVertexFactory*, VertexFactory, this,
		const FGeneratedMeshVertexBuffer*, VertexBuffer, VertexBuffer,
		{
		// Initialize the vertex factory's stream components.
		DataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
		NewData.TextureCoordinates.Add(
			FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate), sizeof(FDynamicMeshVertex), VET_Float2)
			);
		NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
		NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
		VertexFactory->SetData(NewData);
		});
}

FGeneratedMeshSceneProxy::FGeneratedMeshSceneProxy(UCubiquityMeshComponent* Component)
	: FPrimitiveSceneProxy(Component)
	, MaterialRelevance(Component->GetMaterialRelevance(ERHIFeatureLevel::SM4))
{
	//UE_LOG(CubiquityLog, Log, TEXT("Recreating proxy"));
	//UE_LOG(CubiquityLog, Log, TEXT("Vertices in terrain proxy: %d"), Component->terrainVertices.Num());

	//Copy the buffers in from the component
	VertexBuffer.Vertices = Component->terrainVertices;
	IndexBuffer.Indices = Component->indices;

	// Init vertex factory
	VertexFactory.Init(&VertexBuffer);

	// Enqueue initialization of render resource
	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	// Grab material
	Material = Component->GetMaterial(0);
	if (Material == nullptr)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}
