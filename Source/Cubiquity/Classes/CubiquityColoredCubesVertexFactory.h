// Copyright 2014 Volumes of Fun. All Rights Reserved.

#pragma once

#include "VertexFactory.h"

#include <DynamicMeshBuilder.h>

//#include "CubiquityColoredCubesVertexFactory.generated.h"

struct FColoredCubesVertex
{
	FColoredCubesVertex() {}
	
	FColoredCubesVertex(const FVector& InPosition, const FColor& InColor) :
		Position(InPosition),
		Color(InColor)
	{}

	FColoredCubesVertex(const FDynamicMeshVertex& other) :
		Position(other.Position),
		Color(other.Position)
	{}

	FVector Position;
	FColor Color;
};

/** Vertex Buffer */
class FColoredCubesVertexBuffer : public FVertexBuffer
{
public:
	TArray<FColoredCubesVertex> Vertices;

	virtual void InitRHI()
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FColoredCubesVertex), BUF_Static, CreateInfo);

		// Copy the vertex data into the vertex buffer.
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FColoredCubesVertex), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FColoredCubesVertex));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}

};

/** Index Buffer */
class FColoredCubesIndexBuffer : public FIndexBuffer
{
public:
	TArray<int32> Indices;

	virtual void InitRHI()
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo);

		// Write the indices to the index buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};

/**
* Shader parameters for ColoredCubesVertexFactory.
*/
class FColoredCubesVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	virtual void Bind(const FShaderParameterMap& ParameterMap) override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void SetMesh(FRHICommandList& RHICmdList, FShader* Shader, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, uint32 DataFlags) const override;
	// SpeedTree LOD parameter
	//FShaderParameter LODParameter;
};

class FColoredCubesVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FColoredCubesVertexFactory);
public:

	FColoredCubesVertexFactory()
	{}

	struct DataType : public FVertexFactory::DataType
	{
		FVertexStreamComponent PositionComponent;
		FVertexStreamComponent ColorComponent;
	};

	/** Initialization */
	void Init(const FColoredCubesVertexBuffer* VertexBuffer); //Called by scene proxy to initialise the factory

	/**
	* An implementation of the interface used by TSynchronizedResource to update the resource with new data from the game thread.
	*/
	void SetData(const DataType& InData);

	void InitRHI() override;

	static FColoredCubesVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency);

	/**
	* Should we cache the material's shadertype on this platform with this vertex factory?
	*/
	static bool ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

protected:
	DataType Data;
};



class UCubiquityMeshComponent; //Forward declare

class FColoredCubesSceneProxy : public FPrimitiveSceneProxy
{
public:

	FColoredCubesSceneProxy(UCubiquityMeshComponent* Component);

	virtual ~FColoredCubesSceneProxy();

	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	virtual bool CanBeOccluded() const override { return !MaterialRelevance.bDisableDepthTest; };

	virtual uint32 GetMemoryFootprint(void) const { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:

	UMaterialInterface* Material;
	FColoredCubesVertexBuffer VertexBuffer;
	FColoredCubesIndexBuffer IndexBuffer;
	FColoredCubesVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;
};
