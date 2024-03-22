#include "vcepch.h"
#include "FoliageRenderer.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Components/FoliageComponent.h"


namespace VeryCoolEngine {
	FoliageRenderer::FoliageRenderer()
	{
		m_pxFoliageCommandBuffer = RendererAPI::CommandBuffer::Create();

		m_pxQuadMesh = Mesh::GenerateQuad(50);
		m_pxQuadMesh->PlatformInit();
	}

	FoliageRenderer::~FoliageRenderer()
	{
		delete m_pxFoliageCommandBuffer;
	}

	void FoliageRenderer::RenderFoliage(Scene* pxScene) {
		Application* app = Application::GetInstance();
		Renderer* pxRenderer = Renderer::GetRendererInstance();

		m_pxFoliageCommandBuffer->BeginRecording();

		m_pxFoliageCommandBuffer->SubmitTargetSetup(pxRenderer->m_xTargetSetups.at("RenderToTextureNoClear"));


		m_pxFoliageCommandBuffer->SetPipeline(pxRenderer->m_xPipelines.at("Foliage"));

		m_pxFoliageCommandBuffer->BeginBind(RendererAPI::BINDING_FREQUENCY_PER_FRAME);

		m_pxFoliageCommandBuffer->BindBuffer(app->_pCameraUBO->ppBuffers[pxRenderer->m_currentFrame], 0, 0);

		m_pxFoliageCommandBuffer->SetVertexBuffer(m_pxQuadMesh->m_pxVertexBuffer, 0);
		m_pxFoliageCommandBuffer->SetIndexBuffer(m_pxQuadMesh->m_pxIndexBuffer);

		m_pxFoliageCommandBuffer->BeginBind(RendererAPI::BINDING_FREQUENCY_PER_DRAW);

		for (FoliageComponent* pxFoliageComponent : pxScene->GetAllOfComponentType<FoliageComponent>()) {

			struct FoliagePushConstant {
				glm::mat4 xMatrix;
			} xPushConstant;

			xPushConstant.xMatrix = glm::translate(pxFoliageComponent->m_xPos);

			m_pxFoliageCommandBuffer->PushConstant(&xPushConstant, sizeof(xPushConstant));

			FoliageMaterial* pxMaterial = pxFoliageComponent->m_pxMaterial;

			m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxAlbedo, 0, 1);
			m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxBumpMap, 1, 1);
			m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxRoughnessTex, 2, 1);
			m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxAlphaTex, 3, 1);
			m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxTranslucencyTex, 4, 1);
			m_pxFoliageCommandBuffer->BindTexture(pxMaterial->m_pxHeightmapTex, 5, 1);

			m_pxFoliageCommandBuffer->Draw(m_pxQuadMesh->m_uNumIndices, m_pxQuadMesh->m_uNumInstances);

		}

		m_pxFoliageCommandBuffer->EndRecording(RENDER_ORDER_FOLIAGE);
	}
}