// Copyright 2014 Volumes of Fun. All Rights Reserved.

#include "CubiquityPluginPrivatePCH.h"

#include "CubiquityColoredCubesVertexFactory.h"

#include "CubiquityMeshComponent.h"

void FColoredCubesVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	//LODParameter.Bind(ParameterMap, TEXT("SpeedTreeLODInfo"));
}

void FColoredCubesVertexFactoryShaderParameters::Serialize(FArchive& Ar)
{
	//Ar << LODParameter;
}

void FColoredCubesVertexFactoryShaderParameters::SetMesh(FRHICommandList& RHICmdList, FShader* Shader, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, uint32 DataFlags) const
{
	/*if (View.Family != NULL && View.Family->Scene != NULL)
	{
		FUniformBufferRHIParamRef SpeedTreeUniformBuffer = View.Family->Scene->GetSpeedTreeUniformBuffer(VertexFactory);
		if (SpeedTreeUniformBuffer != NULL)
		{
			SetUniformBufferParameter(RHICmdList, Shader->GetVertexShader(), Shader->GetUniformBufferParameter<FSpeedTreeUniformParameters>(), SpeedTreeUniformBuffer);

			if (LODParameter.IsBound())
			{
				FVector LODData(BatchElement.MinScreenSize, BatchElement.MaxScreenSize, BatchElement.MaxScreenSize - BatchElement.MinScreenSize);
				SetShaderValue(RHICmdList, Shader->GetVertexShader(), LODParameter, LODData);
			}
		}
	}*/
}

void FColoredCubesVertexFactory::Init(const FColoredCubesVertexBuffer* VertexBuffer)
{
	check(!IsInRenderingThread());

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		InitGeneratedMeshVertexFactory,
		FColoredCubesVertexFactory*, VertexFactory, this,
		const FColoredCubesVertexBuffer*, VertexBuffer, VertexBuffer,
		{
		// Initialize the vertex factory's stream components.
		DataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FColoredCubesVertex, Position, VET_Float3);
		NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FColoredCubesVertex, Color, VET_Color);
		VertexFactory->SetData(NewData);
		});
}

void FColoredCubesVertexFactory::InitRHI()
{
	FVertexDeclarationElementList Elements;
	
	Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));

	Elements.Add(AccessStreamComponent(Data.ColorComponent, 1));

	check(Streams.Num() > 0);

	InitDeclaration(Elements, Data);

	check(IsValidRef(GetDeclaration()));
}

void FColoredCubesVertexFactory::SetData(const DataType& InData)
{
	check(IsInRenderingThread());
	Data = InData;
	UpdateRHI();
}

FColoredCubesVertexFactoryShaderParameters* FColoredCubesVertexFactory::ConstructShaderParameters(EShaderFrequency ShaderFrequency)
{
	if (ShaderFrequency == SF_Vertex)
	{
		return new FColoredCubesVertexFactoryShaderParameters();
	}

	return NULL;
}

bool FColoredCubesVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return true;
}

//The string here refers to the file "CubiquityColoredCubesVertexFactory.usf"
IMPLEMENT_VERTEX_FACTORY_TYPE(FColoredCubesVertexFactory, "CubiquityColoredCubesVertexFactory", true, true, true, true, true);



FColoredCubesSceneProxy::FColoredCubesSceneProxy(UCubiquityMeshComponent* Component)
	: FPrimitiveSceneProxy(Component)
	, MaterialRelevance(Component->GetMaterialRelevance(ERHIFeatureLevel::SM4))
{
	//UE_LOG(CubiquityLog, Log, TEXT("Recreating proxy"));
	//UE_LOG(CubiquityLog, Log, TEXT("Vertices in colored cubes proxy: %d"), Component->coloredCubesVertices.Num());
	//Copy the buffers in from the component
	for (const auto& vertex : Component->coloredCubesVertices)
	{
		VertexBuffer.Vertices.Add(vertex); //Do the conversion one vertex at a time
	}
	IndexBuffer.Indices = Component->indices;

	// Init vertex factory
	VertexFactory.Init(&VertexBuffer);

	// Enqueue initialization of render resource
	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	// Grab material
	Material = Component->GetMaterial(0);
	if (Material == NULL)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

FColoredCubesSceneProxy::~FColoredCubesSceneProxy()
{
	VertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}

void FColoredCubesSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_GeneratedMeshSceneProxy_DrawDynamicElements);

	const bool bWireframe = View->Family->EngineShowFlags.Wireframe;

	FColoredMaterialRenderProxy WireframeMaterialInstance(
		WITH_EDITOR ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
		//GEngine->WireframeMaterial->GetRenderProxy(IsSelected()),
		FLinearColor(0, 0.5f, 1.f)
		);

	FMaterialRenderProxy* MaterialProxy = NULL;
	if (bWireframe)
	{
		MaterialProxy = &WireframeMaterialInstance;
	}
	else
	{
		MaterialProxy = Material->GetRenderProxy(IsSelected());
	}

	// Draw the mesh.
	FMeshBatch Mesh;
	FMeshBatchElement& BatchElement = Mesh.Elements[0];
	BatchElement.IndexBuffer = &IndexBuffer;
	Mesh.bWireframe = bWireframe;
	Mesh.VertexFactory = &VertexFactory;
	Mesh.MaterialRenderProxy = MaterialProxy;
	BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
	Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
	Mesh.Type = PT_TriangleList;
	Mesh.DepthPriorityGroup = SDPG_World;
	PDI->DrawMesh(Mesh);
}

FPrimitiveViewRelevance FColoredCubesSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	return Result;
}
