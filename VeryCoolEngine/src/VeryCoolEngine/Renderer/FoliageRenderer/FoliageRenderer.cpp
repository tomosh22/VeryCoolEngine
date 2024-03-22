#include "vcepch.h"
#include "FoliageRenderer.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Components/FoliageComponent.h"

#define MAX_FOLIAGE_QUADS 256


namespace VeryCoolEngine {
	FoliageRenderer::FoliageRenderer()
	{
		Application* pxApp = Application::GetInstance();
		m_pxFoliageCommandBuffer = RendererAPI::CommandBuffer::Create();

		m_pxQuadMesh = Mesh::GenerateQuad(50);
		m_pxQuadMesh->m_pxInstanceBuffer = VertexBuffer::Create(nullptr, sizeof(glm::vec3) * MAX_FOLIAGE_QUADS, false);

		BufferLayout xInstancedLayout;
		xInstancedLayout.GetElements().push_back(BufferElement(
			ShaderDataType::Float3,
			"a_InstancePosition",
			false,
			true,
			1,
			nullptr,
			0
		));
		xInstancedLayout.CalculateOffsetsAndStrides();
		m_pxQuadMesh->m_pxInstanceBuffer->SetLayout(xInstancedLayout);

		m_pxQuadMesh->PlatformInit();

		m_pxFoliageShader = Shader::Create("vulkan/Foliage/foliageVert.spv", "vulkan/Foliage/foliageFrag.spv");

		pxApp->m_xPipelineSpecs.insert(
			{ "Foliage",
					PipelineSpecification(
					"Foliage",
					m_pxQuadMesh,
					m_pxFoliageShader,
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					"RenderToTextureNoClear",
					true,
					false,
					{
						{1,0},
						{0,6}
					}
					)
			});
	}

	FoliageRenderer::~FoliageRenderer()
	{
		delete m_pxFoliageCommandBuffer;
		delete m_pxQuadMesh;
	}

	void FoliageRenderer::UpdateInstanceBuffer(Scene* pxScene)
	{
		std::vector<FoliageComponent*> xComponents = pxScene->GetAllOfComponentType<FoliageComponent>();

		VCE_ASSERT(xComponents.size() <= MAX_FOLIAGE_QUADS, "Too many foliage quads");

		std::vector<glm::vec3> xPositions(xComponents.size());
		for (uint32_t i = 0; i < xComponents.size(); i++)
			xPositions.at(i) = xComponents[i]->m_xPos;

		m_pxQuadMesh->m_pxInstanceBuffer->m_pxVertexBuffer->UploadData(xPositions.data(), sizeof(glm::vec3) * xPositions.size());
		
		m_pxQuadMesh->m_uNumInstances = xComponents.size();
	}

	void FoliageRenderer::RenderFoliage(Scene* pxScene) {
		Application* app = Application::GetInstance();
		Renderer* pxRenderer = Renderer::GetRendererInstance();

		UpdateInstanceBuffer(pxScene);

		m_pxFoliageCommandBuffer->BeginRecording();

		m_pxFoliageCommandBuffer->SubmitTargetSetup(pxRenderer->m_xTargetSetups.at("RenderToTextureNoClear"));

		m_pxFoliageCommandBuffer->SetPipeline(pxRenderer->m_xPipelines.at("Foliage"));

		m_pxFoliageCommandBuffer->SetVertexBuffer(m_pxQuadMesh->m_pxVertexBuffer, 0);
		m_pxFoliageCommandBuffer->SetVertexBuffer(m_pxQuadMesh->m_pxInstanceBuffer, 1);
		m_pxFoliageCommandBuffer->SetIndexBuffer(m_pxQuadMesh->m_pxIndexBuffer);

		m_pxFoliageCommandBuffer->BeginBind(RendererAPI::BINDING_FREQUENCY_PER_FRAME);

		m_pxFoliageCommandBuffer->BindBuffer(app->_pCameraUBO->ppBuffers[pxRenderer->m_currentFrame], 0, 0);

		//#TO_TODO: how do I want to handle foliage textures now?
		m_pxFoliageCommandBuffer->BeginBind(RendererAPI::BINDING_FREQUENCY_PER_DRAW);
		
		FoliageMaterial* pxMaterial = pxScene->GetAllOfComponentType<FoliageComponent>().back()->m_pxMaterial;

		m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxAlbedo, 0, 1);
		m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxBumpMap, 1, 1);
		m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxRoughnessTex, 2, 1);
		m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxAlphaTex, 3, 1);
		m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxTranslucencyTex, 4, 1);
		m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxHeightmapTex, 5, 1);

		m_pxFoliageCommandBuffer->Draw(m_pxQuadMesh->m_uNumIndices, m_pxQuadMesh->m_uNumInstances);

		m_pxFoliageCommandBuffer->EndRecording(RENDER_ORDER_FOLIAGE);
	}
}