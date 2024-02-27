#include "vcepch.h"
#include "VulkanRenderer.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Platform/Vulkan/VulkanMesh.h"

#include "Platform/Vulkan/VulkanManagedUniformBuffer.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanPipelineBuilder.h"
#include "Platform/Vulkan/VulkanShader.h"

#include "VulkanPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"

#include <imgui.h>
#include "backends/imgui_impl_vulkan.h"

#include "VeryCoolEngine/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include "VulkanMaterial.h"

#include "reactphysics3d/reactphysics3d.h"
#include "VeryCoolEngine/Components/ModelComponent.h"

#include "VeryCoolEngine/Renderer/AsyncLoader.h"


using namespace VeryCoolEngine;

VulkanRenderer* VulkanRenderer::s_pInstance = nullptr;

VulkanRenderer::VulkanRenderer() {
	InitWindow();
	InitVulkan();

	m_pxRendererAPI = new RendererAPI;

	m_pxCopyToFramebufferCommandBuffer = new VulkanCommandBuffer(false);
	m_pxSkyboxCommandBuffer = new VulkanCommandBuffer(false);
	m_pxOpaqueMeshesCommandBuffer = new VulkanCommandBuffer(false);
	m_pxSkinnedMeshesCommandBuffer = new VulkanCommandBuffer(false);
	m_pxFoliageCommandBuffer = new VulkanCommandBuffer(false);

	AsyncLoader::g_pxAsyncLoaderCommandBuffer = new VulkanCommandBuffer(true);


	//TODO: delete me
	Application::GetInstance()->m_pxFoliageMaterial->PlatformInit();

	Application::GetInstance()->renderInitialised = true;
}

void VulkanRenderer::InitWindow() {
	Application* app = Application::GetInstance();
	((WindowsWindow*)app->_window)->Init(WindowProperties());
}

void VulkanRenderer::InitialiseAssets() {
	Application* app = Application::GetInstance();

	
	CreateDescriptorPool(false);

	

	for (auto it = app->m_xPipelineSpecs.begin(); it != app->m_xPipelineSpecs.end(); it++) {
		m_xPipelines.insert({ it->first,VulkanPipelineBuilder::FromSpecification(it->second) });
	}



	Scene* pxScene = app->m_pxCurrentScene;

	VCE_ASSERT(pxScene != nullptr, "Null scene");

	

	for (ModelComponent* pxModelComponent : app->m_pxCurrentScene->GetAllOfComponentType<ModelComponent>()) {
		for (Mesh* pMesh : pxModelComponent->GetModel()->m_apxMeshes) {
			pMesh->PlatformInit();
		}
	}
}

void VulkanRenderer::CleanupAssets() {
	Application* app = Application::GetInstance();
	Scene* pxScene = app->m_pxCurrentScene;


	for (auto it = m_xPipelines.begin(); it != m_xPipelines.end(); it++)
		delete it->second;
	m_xPipelines.clear();

	m_device.destroyDescriptorPool(m_descriptorPool);
}

void VeryCoolEngine::VulkanRenderer::WaitDeviceIdle()
{
	m_device.waitIdle();
}

void VulkanRenderer::InitVulkan() {
	VulkanRenderer::s_pInstance = this;
	Application* app = Application::GetInstance();

	BoilerplateInit();
	
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	app->_pLightUBO = ManagedUniformBuffer::Create(sizeof(RendererAPI::Light) * RendererAPI::g_uMaxLights, MAX_FRAMES_IN_FLIGHT, 1);
	app->m_pxMiscMeshRenderDataUBO = ManagedUniformBuffer::Create(sizeof(Application::MeshRenderData), MAX_FRAMES_IN_FLIGHT, 2);

	for (Shader* pxShader : app->_shaders) pxShader->PlatformInit();

	
	
}

#ifdef VCE_DEFERRED_SHADING
void VulkanRenderer::SetupDeferredShading() {
	for (uint32_t i = 0; i < m_swapChainImages.size(); i++) {
		m_apxGBufferDiffuse.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxGBufferNormals.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxGBufferMaterial.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxGBufferDepth.emplace_back(VulkanTexture2D::CreateDepthAttachment(m_width, m_height));
		m_apxDeferredDiffuse.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxDeferredSpecular.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
	}
}
#endif

void VulkanRenderer::MainLoop() {
	glfwPollEvents();

	Application* app = Application::GetInstance();

	if (app->m_bSkipFrame || app->m_bWantToResetScene)
		return;

	RendererScene* scene = app->m_pxRendererScene;

	//this is just so the render thread doesn't claim the mutex before the main thread when the game first boots, should probably have a better way of handling this
	while (true) {
		std::this_thread::yield();
		if (scene->ready)break;
	}
	app->sceneMutex.lock();

	ProfilingBeginFrame();

	BeginScene(scene);

	

	DrawFrame(app->m_pxRendererScene);

	ProfilingEndFrame();

	app->sceneMutex.unlock();

	//#TO_TODO: really don't like that this is here
	Input::ResetPressedKeys();
}

void VulkanRenderer::CopyToFramebuffer() {
	Application* app = Application::GetInstance();

#ifdef VCE_USE_EDITOR
	app->_pImGuiLayer->Begin();
	app->_pImGuiLayer->OnImGuiRender();
#endif
	



#pragma region gbuffer
	
#ifdef VCE_DEFERRED_SHADING
	m_pxCopyToFramebufferCommandBuffer->SubmitTargetSetup(RendererAPI::s_xGBufferTargetSetup);

	//BeginGBufferRenderPass(commandBuffer, imageIndex);
	VulkanPipeline* pxGBufferPipeline = m_xPipelines.at("GBuffer");

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pxGBufferPipeline->m_xPipeline);

	for (Mesh* mesh : scene->m_axPipelineMeshes.at(pxGBufferPipeline->m_strName)) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		pxVulkanMesh->BindToCmdBuffer(commandBuffer);

		std::vector<vk::DescriptorSet> axSets;
		for (const vk::DescriptorSet set : pxGBufferPipeline->m_axBufferDescSets)
			axSets.push_back(set);
		if (pxVulkanMesh->GetTexture() != nullptr)
			axSets.push_back(pxVulkanMesh->m_xTexDescSet);

		pxGBufferPipeline->BindDescriptorSets(commandBuffer, axSets, vk::PipelineBindPoint::eGraphics, 0);

		if (pxGBufferPipeline->bUsePushConstants) {
			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, 0, sizeof(glm::mat4), (void*)&mesh->m_xTransform);

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4), sizeof(glm::vec3), (void*)&m_xOverrideNormal);
			int uValue = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3), sizeof(uint32_t), (void*)&uValue);

			int uValueTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t), sizeof(uint32_t), (void*)&uValueTess);

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(uint32_t), sizeof(float), (void*)&app->_pRenderer->m_fPhongTessFactor);
			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(float), sizeof(float), (void*)&app->_pRenderer->m_uTessLevel);
		}

		commandBuffer.drawIndexed(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);
	}

	commandBuffer.endRenderPass();
#endif

#pragma endregion

	m_pxCopyToFramebufferCommandBuffer->BeginRecording();

	m_pxCopyToFramebufferCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("CopyToFramebuffer"));

	m_pxCopyToFramebufferCommandBuffer->SetPipeline(m_xPipelines.at("CopyToFramebuffer"));

	VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->m_pxQuadModel->m_apxMeshes.back());
	m_pxCopyToFramebufferCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
	m_pxCopyToFramebufferCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

	m_pxCopyToFramebufferCommandBuffer->BindTexture(m_apxEditorSceneTexs[m_currentFrame], 0, 0);
	
	m_pxCopyToFramebufferCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);

	
	
#ifdef VCE_USE_EDITOR

	m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer().endRenderPass();

	UpdateImageDescriptor(m_axFramebufferTexDescSet[m_currentFrame], 0, 0, m_apxEditorSceneTexs[m_currentFrame]->m_xImageView, m_xDefaultSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	BeginImguiRenderPass(m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer(), m_currentFrame);
	
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer());
	app->_pImGuiLayer->End();
#endif

	
	m_pxCopyToFramebufferCommandBuffer->EndRecording(RENDER_ORDER_COPY_TO_FRAMEBUFFER);
	
}

void VulkanRenderer::DrawSkybox() {
	Application* app = Application::GetInstance();

	m_pxSkyboxCommandBuffer->BeginRecording();

	m_pxSkyboxCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTextureClear"));

	VulkanPipeline* pxSkyboxPipeline = m_xPipelines.at("Skybox");
	m_pxSkyboxCommandBuffer->SetPipeline(&pxSkyboxPipeline->m_xPipeline);

	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxSkyboxCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0, 0);

	VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->m_pxQuadModel->m_apxMeshes.back());
	m_pxSkyboxCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
	m_pxSkyboxCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);
	m_pxSkyboxCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);


	m_pxSkyboxCommandBuffer->EndRecording(RENDER_ORDER_SKYBOX);
}

void VulkanRenderer::DrawOpaqueMeshes() {
	Application* app = Application::GetInstance();

	m_pxOpaqueMeshesCommandBuffer->BeginRecording();

	m_pxOpaqueMeshesCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTextureNoClear"));


	m_pxOpaqueMeshesCommandBuffer->SetPipeline(&m_xPipelines.at("Meshes")->m_xPipeline);

	Application::MeshRenderData xMeshRenderData;

	xMeshRenderData.xOverrideNormal = m_xOverrideNormal;
	xMeshRenderData.uUseBumpMap = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;
	xMeshRenderData.uVisualiseNormals = app->_pRenderer->m_bVisualiseNormals ? 1 : 0;
	xMeshRenderData.uUsePhongTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;
	xMeshRenderData.fPhongTessFactor = app->_pRenderer->m_fPhongTessFactor;
	xMeshRenderData.uTessLevel = app->_pRenderer->m_uTessLevel;

	app->m_pxMiscMeshRenderDataUBO->UploadData(&xMeshRenderData, sizeof(Application::MeshRenderData), m_currentFrame, 0);

	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0, 0);

	VulkanManagedUniformBuffer* pxLightUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pLightUBO);
	m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxLightUBO->ppBuffers[m_currentFrame], 1, 0);


	VulkanManagedUniformBuffer* pxMiscMeshRenderDataUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->m_pxMiscMeshRenderDataUBO);
	m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxMiscMeshRenderDataUBO->ppBuffers[m_currentFrame], 2, 0);


	for (VCEModel* pxModel : app->m_pxRendererScene->GetModelsInPipeline("Meshes")) {
		struct OpaqueMeshPushConstant {
			glm::mat4 xMatrix;
			int bSelected;
		} xPushConstant;
		xPushConstant.xMatrix = pxModel->m_xModelMat;
		xPushConstant.bSelected = pxModel == app->m_pxSelectedModel ? 1 : 0;

		for (Mesh* pxMesh : pxModel->m_apxMeshes) {
			VCE_ASSERT(pxMesh->m_bInitialised, "Mesh not initalised");
			VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(pxMesh);
			m_pxOpaqueMeshesCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
			m_pxOpaqueMeshesCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

			m_pxOpaqueMeshesCommandBuffer->BindMaterial(pxMesh->m_pxMaterial, 1);



			m_pxOpaqueMeshesCommandBuffer->PushConstant(&xPushConstant, sizeof(OpaqueMeshPushConstant));

			m_pxOpaqueMeshesCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances);
		}
	}


	m_pxOpaqueMeshesCommandBuffer->EndRecording(RENDER_ORDER_OPAQUE_MESHES);
}

void VulkanRenderer::DrawSkinnedMeshes() {
	Application* app = Application::GetInstance();

	m_pxSkinnedMeshesCommandBuffer->BeginRecording();

	m_pxSkinnedMeshesCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTextureNoClear"));


	m_pxSkinnedMeshesCommandBuffer->SetPipeline(&m_xPipelines.at("SkinnedMeshes")->m_xPipeline);

	Application::MeshRenderData xMeshRenderData;

	xMeshRenderData.xOverrideNormal = m_xOverrideNormal;
	xMeshRenderData.uUseBumpMap = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;
	xMeshRenderData.uVisualiseNormals = app->_pRenderer->m_bVisualiseNormals ? 1 : 0;
	xMeshRenderData.uUsePhongTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;
	xMeshRenderData.fPhongTessFactor = app->_pRenderer->m_fPhongTessFactor;
	xMeshRenderData.uTessLevel = app->_pRenderer->m_uTessLevel;

	app->m_pxMiscMeshRenderDataUBO->UploadData(&xMeshRenderData, sizeof(Application::MeshRenderData), m_currentFrame, 0);


	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0, 0);

	VulkanManagedUniformBuffer* pxLightUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pLightUBO);
	m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxLightUBO->ppBuffers[m_currentFrame], 1, 0);




	VulkanManagedUniformBuffer* pxMiscMeshRenderDataUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->m_pxMiscMeshRenderDataUBO);
	m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxMiscMeshRenderDataUBO->ppBuffers[m_currentFrame], 2, 0);

	for (VCEModel* pxModel : app->m_pxRendererScene->GetModelsInPipeline("SkinnedMeshes")) {
		for (Mesh* pxMesh : pxModel->m_apxMeshes) {
			VCE_ASSERT(pxMesh->m_bInitialised, "Mesh not initalised");
			VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(pxMesh);
			m_pxSkinnedMeshesCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
			m_pxSkinnedMeshesCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

			pxVulkanMesh->m_pxBoneBuffer->UploadData(pxVulkanMesh->m_xBoneMats.data(), pxVulkanMesh->m_xBoneMats.size() * sizeof(glm::mat4), m_currentFrame);

			m_pxSkinnedMeshesCommandBuffer->BindMaterial(pxMesh->m_pxMaterial, 1);

			m_pxSkinnedMeshesCommandBuffer->BindAnimation(pxMesh, 2);

			RendererAPI::MeshPushConstantData xPushConstants;
			xPushConstants.m_xModelMat = pxModel->m_xModelMat;
			xPushConstants.m_uAnimate = bAnimate ? 1 : 0;
			xPushConstants.m_fAlpha = fAnimAlpha;

			m_pxSkinnedMeshesCommandBuffer->PushConstant(&xPushConstants, sizeof(RendererAPI::MeshPushConstantData));


			m_pxSkinnedMeshesCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances);
		}
	}

	m_pxSkinnedMeshesCommandBuffer->EndRecording(RENDER_ORDER_SKINNED_MESHES);
}

void VulkanRenderer::DrawFoliage() {
	Application* app = Application::GetInstance();

	m_pxFoliageCommandBuffer->BeginRecording();

	m_pxFoliageCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTextureNoClear"));


	m_pxFoliageCommandBuffer->SetPipeline(&m_xPipelines.at("Foliage")->m_xPipeline);



	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxFoliageCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0, 0);

	
	VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->m_pxFoliageModel->m_apxMeshes.back());
	m_pxFoliageCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer,0);
	m_pxFoliageCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxInstanceBuffer,1);
	m_pxFoliageCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

	VulkanFoliageMaterial* pxVkMaterial = dynamic_cast<VulkanFoliageMaterial*>(app->m_pxFoliageMaterial);

	m_pxFoliageCommandBuffer->m_xCurrentCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pxFoliageCommandBuffer->m_pxCurrentPipeline->m_xPipelineLayout, 1, 1, &pxVkMaterial->m_xDescSet, 0, nullptr);

	m_pxFoliageCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances);
	
	

	m_pxFoliageCommandBuffer->EndRecording(RENDER_ORDER_FOLIAGE);
}

void VulkanRenderer::DrawFrame(RendererScene* scene) {
	if (m_bShouldResize) {
		RecreateSwapChain();
		m_bShouldResize = false;
		m_currentFrame = 0;
		return;
	}
	uint32_t iImageIndex = AcquireSwapchainImage();
	if (iImageIndex == -1) {
		RecreateSwapChain();
		return;
	}

	static uint32_t uFrameCount = 0;
	Application* pxApp = Application::GetInstance();
	if(uFrameCount > 1000)
		pxApp->m_xAsyncLoader.ProcessPendingStreams_MainThread();
	uFrameCount++;


	DrawSkybox();

	DrawOpaqueMeshes();

	DrawSkinnedMeshes();

	//DrawFoliage();

#ifdef VCE_USE_EDITOR
	//TODO: I'm guessing the editor scene textures are being initalised in the wrong format
	static uint32_t uNumFramesPassed = 0;
	if (uNumFramesPassed < MAX_FRAMES_IN_FLIGHT) {
		ImageTransitionBarrier(m_apxEditorSceneTexs[m_currentFrame]->m_xImage, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader);
		uNumFramesPassed++;
	}
#endif

	CopyToFramebuffer();
	
	m_pxRendererAPI->Platform_SubmitCmdBuffers();

	Present(iImageIndex, &m_renderFinishedSemaphores[m_currentFrame], 1);


	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VeryCoolEngine::VulkanRenderer::BeginScene(RendererScene* scene)
{
	Application* app = Application::GetInstance();

	RendererAPI::FrameConstants xFrameConstants;
	xFrameConstants.m_xViewMat = scene->camera->BuildViewMatrix();
	xFrameConstants.m_xProjMat = scene->camera->BuildProjectionMatrix();
	xFrameConstants.m_xViewProjMat = xFrameConstants.m_xProjMat * xFrameConstants.m_xViewMat;
	glm::vec3 tempCamPos = scene->camera->GetPosition();
	xFrameConstants.m_xCamPos = { tempCamPos.x, tempCamPos.y, tempCamPos.z,0 };
	app->_pCameraUBO->UploadData(&xFrameConstants, sizeof(RendererAPI::FrameConstants), m_currentFrame, 0);


	RendererAPI::LightData xLightData;
	memcpy(xLightData.GetLightsPtr(), scene->lights.data(), sizeof(RendererAPI::Light) * scene->lights.size());
	xLightData.CalculateNumLights();

	app->_pLightUBO->UploadData(&xLightData, xLightData.GetUploadSize(), m_currentFrame, 0);

}

void VeryCoolEngine::VulkanRenderer::RenderThreadFunction()
{
	Application* app = Application::GetInstance();
#ifdef VCE_USE_EDITOR
	app->_pImGuiLayer = new ImGuiLayer();
	app->PushOverlay(app->_pImGuiLayer);
#endif
	while (app->_running) {
		MainLoop();
	}
}

void RendererAPI::Platform_SubmitCmdBuffers() {
	VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();

	vk::PipelineStageFlags eWaitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eTransfer;

	AsyncLoader::g_xAsyncLoaderMutex.lock();

	std::vector<vk::CommandBuffer> xPlatformCmdBufs;
	for (uint32_t i = 0; i < RENDER_ORDER_MAX; i++){
		for (void* pCmdBuf : s_axCmdBuffersToSubmit[i]) {
			vk::CommandBuffer& xBuf = *reinterpret_cast<vk::CommandBuffer*>(pCmdBuf);
			xPlatformCmdBufs.push_back(xBuf);
		}
	}

	vk::SubmitInfo xSubmitInfo = vk::SubmitInfo()
		.setCommandBufferCount(xPlatformCmdBufs.size())
		.setPCommandBuffers(xPlatformCmdBufs.data())
		.setPWaitSemaphores(&pxRenderer->GetCurrentImageAvailableSem())
		.setPSignalSemaphores(&pxRenderer->GetCurrentRenderCompleteSem())
		.setWaitSemaphoreCount(1)
		.setSignalSemaphoreCount(1)
		.setWaitDstStageMask(eWaitStages);


	pxRenderer->GetGraphicsQueue().submit(xSubmitInfo, pxRenderer->GetCurrentInFlightFence());


	//TODO: put this in end frame when I eventually write it
	for (uint32_t i = 0; i < RENDER_ORDER_MAX; i++) {
		s_axCmdBuffersToSubmit[i].clear();
	}

	AsyncLoader::g_xAsyncLoaderMutex.unlock();
}

void VeryCoolEngine::VulkanRenderer::ProfilingBeginFrame() {
	m_uNumDrawCalls = 0;
}

void VeryCoolEngine::VulkanRenderer::RecordDrawCall() {
	m_uNumDrawCalls++;
}

void VeryCoolEngine::VulkanRenderer::ProfilingEndFrame() {
}