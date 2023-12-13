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


using namespace VeryCoolEngine;

VulkanRenderer* VulkanRenderer::s_pInstance = nullptr;

VulkanRenderer::VulkanRenderer() {
	InitWindow();
	InitVulkan();

	m_pxRendererAPI = new RendererAPI;

	m_pxCopyToFramebufferCommandBuffer = new VulkanCommandBuffer;
	m_pxSkyboxCommandBuffer = new VulkanCommandBuffer;
	m_pxOpaqueMeshesCommandBuffer = new VulkanCommandBuffer;
	m_pxSkinnedMeshesCommandBuffer = new VulkanCommandBuffer;




#ifdef VCE_DEFERRED_SHADING
	RendererAPI::s_xGBufferTargetSetup = CreateGBufferTarget();
#endif
	m_xTargetSetups.insert({ "RenderToTexture", CreateRenderToTextureTarget() });
	m_xTargetSetups.insert({ "CopyToFramebuffer", CreateFramebufferTarget() });
}

void VulkanRenderer::InitWindow() {
	Application* app = Application::GetInstance();
	((WindowsWindow*)app->_window)->Init(WindowProperties());
}

void VulkanRenderer::InitVulkan() {
	VulkanRenderer::s_pInstance = this;
	Application* app = Application::GetInstance();

	BoilerplateInit();


	
	for (Mesh* pMesh : app->_meshes) {
		pMesh->PlatformInit();
		pMesh->GetShader()->PlatformInit();
	}
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	app->_pLightUBO = ManagedUniformBuffer::Create(sizeof(Light) * _sMAXLIGHTS, MAX_FRAMES_IN_FLIGHT, 1);
	app->m_pxPushConstantUBO = ManagedUniformBuffer::Create(sizeof(Light) * _sMAXLIGHTS, MAX_FRAMES_IN_FLIGHT, 2);

	for (Shader* pxShader : app->_shaders) pxShader->PlatformInit();
	//for (Texture* pxTex : app->_textures) pxTex->PlatformInit();

	for (auto it = app->m_xPipelineSpecs.begin(); it != app->m_xPipelineSpecs.end(); it++) {
		m_xPipelines.insert({ it->first,VulkanPipelineBuilder::FromSpecification(it->second) });
	}
	
	

	Application::GetInstance()->renderInitialised = true;
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

	Scene* scene = app->scene;
	while (true) {
		std::this_thread::yield();
		if (scene->ready)break;//#todo implement mutex here
	}
	app->sceneMutex.lock();


	BeginScene(scene);

	

	DrawFrame(app->scene);

	

	app->sceneMutex.unlock();
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

	m_pxCopyToFramebufferCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("CopyToFramebuffer"), true);

	m_pxCopyToFramebufferCommandBuffer->SetPipeline(m_xPipelines.at("CopyToFramebuffer"));

	VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->m_pxQuadMesh);
	m_pxCopyToFramebufferCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
	m_pxCopyToFramebufferCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

	m_pxCopyToFramebufferCommandBuffer->BindTexture(m_apxEditorSceneTexs[m_currentFrame], 0);
	
	m_pxCopyToFramebufferCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);
	m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer().endRenderPass();
	
#ifdef VCE_USE_EDITOR

	UpdateImageDescriptor(m_axFramebufferTexDescSet[m_currentFrame], 0, 0, m_apxEditorSceneTexs[m_currentFrame]->m_xImageView, m_xDefaultSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	BeginImguiRenderPass(m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer(), m_currentFrame);
	
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer());
	app->_pImGuiLayer->End();
	m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer().endRenderPass();
#endif

	
	m_pxCopyToFramebufferCommandBuffer->EndRecording();
	
}

void VulkanRenderer::DrawSkybox() {
	Application* app = Application::GetInstance();

	m_pxSkyboxCommandBuffer->BeginRecording();

	m_pxSkyboxCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTexture"), false);

	VulkanPipeline* pxSkyboxPipeline = m_xPipelines.at("Skybox");
	m_pxSkyboxCommandBuffer->SetPipeline(&pxSkyboxPipeline->m_xPipeline);

	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxSkyboxCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0);

	VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->m_pxQuadMesh);
	m_pxSkyboxCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
	m_pxSkyboxCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);
	m_pxSkyboxCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);

	m_pxSkyboxCommandBuffer->GetCurrentCmdBuffer().endRenderPass();

	m_pxSkyboxCommandBuffer->EndRecording();
}

void VulkanRenderer::DrawOpaqueMeshes() {
	Application* app = Application::GetInstance();

	m_pxOpaqueMeshesCommandBuffer->BeginRecording();

	m_pxOpaqueMeshesCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTexture"), true);


	m_pxOpaqueMeshesCommandBuffer->SetPipeline(&m_xPipelines.at("Meshes")->m_xPipeline);

	Application::MeshRenderData xMeshRenderData;

	xMeshRenderData.xOverrideNormal = m_xOverrideNormal;
	xMeshRenderData.uUseBumpMap = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;
	xMeshRenderData.uUsePhongTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;
	xMeshRenderData.fPhongTessFactor = app->_pRenderer->m_fPhongTessFactor;
	xMeshRenderData.uTessLevel = app->_pRenderer->m_uTessLevel;

	app->m_pxPushConstantUBO->UploadData(&xMeshRenderData, sizeof(Application::MeshRenderData), m_currentFrame, 0);

	for (Mesh* mesh : app->scene->m_axPipelineMeshes.at("Meshes")) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		m_pxOpaqueMeshesCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
		m_pxOpaqueMeshesCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

		m_pxOpaqueMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetTexture(), 0);
		m_pxOpaqueMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetBumpMap(), 1);
		m_pxOpaqueMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetRoughnessTex(), 2);
		m_pxOpaqueMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetMetallicTex(), 3);
		m_pxOpaqueMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetHeightmapTex(), 4);




		VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
		m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0);

		VulkanManagedUniformBuffer* pxLightUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pLightUBO);
		m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxLightUBO->ppBuffers[m_currentFrame], 1);




		VulkanManagedUniformBuffer* pxPushConstantUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->m_pxPushConstantUBO);
		m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxPushConstantUBO->ppBuffers[m_currentFrame], 2);

		m_pxOpaqueMeshesCommandBuffer->PushConstant(&mesh->m_xTransform._matrix, sizeof(glm::mat4));

		m_pxOpaqueMeshesCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances);
	}

	m_pxOpaqueMeshesCommandBuffer->GetCurrentCmdBuffer().endRenderPass();

	m_pxOpaqueMeshesCommandBuffer->EndRecording();
}

void VulkanRenderer::DrawSkinnedMeshes() {
	Application* app = Application::GetInstance();

	m_pxSkinnedMeshesCommandBuffer->BeginRecording();

	m_pxSkinnedMeshesCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTexture"), false);


	m_pxSkinnedMeshesCommandBuffer->SetPipeline(&m_xPipelines.at("SkinnedMeshes")->m_xPipeline);

	Application::MeshRenderData xMeshRenderData;

	xMeshRenderData.xOverrideNormal = m_xOverrideNormal;
	xMeshRenderData.uUseBumpMap = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;
	xMeshRenderData.uUsePhongTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;
	xMeshRenderData.fPhongTessFactor = app->_pRenderer->m_fPhongTessFactor;
	xMeshRenderData.uTessLevel = app->_pRenderer->m_uTessLevel;

	app->m_pxPushConstantUBO->UploadData(&xMeshRenderData, sizeof(Application::MeshRenderData), m_currentFrame, 0);

	for (Mesh* mesh : app->scene->m_axPipelineMeshes.at("SkinnedMeshes")) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		m_pxSkinnedMeshesCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
		m_pxSkinnedMeshesCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

		m_pxSkinnedMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetTexture(), 0);
		m_pxSkinnedMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetBumpMap(), 1);
		m_pxSkinnedMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetRoughnessTex(), 2);
		m_pxSkinnedMeshesCommandBuffer->BindTexture(pxVulkanMesh->GetMetallicTex(), 3);




		VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
		m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0);

		VulkanManagedUniformBuffer* pxLightUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pLightUBO);
		m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxLightUBO->ppBuffers[m_currentFrame], 1);

		VulkanBuffer* pxBoneBuffer = pxVulkanMesh->m_pxBoneBuffer;
		pxBoneBuffer->UploadData(pxVulkanMesh->m_xBoneMats.data(), pxVulkanMesh->m_xBoneMats.size() * sizeof(glm::mat4));
		m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxBoneBuffer, 3);


		VulkanManagedUniformBuffer* pxPushConstantUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->m_pxPushConstantUBO);
		m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxPushConstantUBO->ppBuffers[m_currentFrame], 2);

		void* pPushConstant = malloc(sizeof(glm::mat4) + sizeof(int) + sizeof(float));
		int uAnimate = bAnimate ? 1 : 0;
		memcpy(pPushConstant, &mesh->m_xTransform._matrix, sizeof(glm::mat4));
		memcpy((char*)pPushConstant + sizeof(glm::mat4), &uAnimate, sizeof(int));
		memcpy((char*)pPushConstant + sizeof(glm::mat4) + sizeof(int), &fAnimAlpha, sizeof(float));

		m_pxSkinnedMeshesCommandBuffer->PushConstant(pPushConstant, sizeof(glm::mat4) + sizeof(int) + sizeof(float));

		free(pPushConstant);

		m_pxSkinnedMeshesCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances);
	}

	m_pxSkinnedMeshesCommandBuffer->GetCurrentCmdBuffer().endRenderPass();

	m_pxSkinnedMeshesCommandBuffer->EndRecording();
}

void VulkanRenderer::DrawFrame(Scene* scene) {
	uint32_t iImageIndex = AcquireSwapchainImage();
	if (iImageIndex == -1) {
		RecreateSwapChain();
		return;
	}

	DrawOpaqueMeshes();

	DrawSkinnedMeshes();

	DrawSkybox();

	

	

	CopyToFramebuffer();
	
	m_pxRendererAPI->Platform_SubmitCmdBuffers();
	

	Present(iImageIndex, &m_renderFinishedSemaphores[m_currentFrame], 1);


	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VeryCoolEngine::VulkanRenderer::BeginScene(Scene* scene)
{
	Application* app = Application::GetInstance();
	const uint32_t camDataSize = sizeof(glm::mat4) * 3 + sizeof(glm::vec4);//4 bytes of padding
	glm::mat4 viewMat = scene->camera->BuildViewMatrix();
	glm::mat4 projMat = scene->camera->BuildProjectionMatrix();
	glm::mat4 viewProjMat = projMat * viewMat;
	glm::vec3 tempCamPos = scene->camera->GetPosition();
	glm::vec4 camPos = { tempCamPos.x, tempCamPos.y, tempCamPos.z,0 };//4 bytes of padding
	char* camData = new char[camDataSize];
	memcpy(camData + sizeof(glm::mat4) * 0, &viewMat[0][0], sizeof(glm::mat4));
	memcpy(camData + sizeof(glm::mat4) * 1, &projMat[0][0], sizeof(glm::mat4));
	memcpy(camData + sizeof(glm::mat4) * 2, &viewProjMat[0][0], sizeof(glm::mat4));
	memcpy(camData + sizeof(glm::mat4) * 3, &camPos[0], sizeof(glm::vec4));
	app->_pCameraUBO->UploadData(camData, camDataSize, m_currentFrame, 0);
	delete[] camData;


	const uint32_t dataSize = (sizeof(unsigned int) * 4) + (sizeof(Light) * scene->lights.size());
	char* data = new char[dataSize];
	unsigned int numLightsWithPadding[4] = { scene->numLights,0,0,0 };//12 bytes of padding

	memcpy(data, numLightsWithPadding, sizeof(unsigned int) * 4);

	memcpy(data + sizeof(unsigned int) * 4, scene->lights.data(), sizeof(Light) * scene->lights.size());
	app->_pLightUBO->UploadData(data, dataSize, m_currentFrame, 0);
	delete[] data;

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

	vk::PipelineStageFlags eWaitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	std::vector<vk::CommandBuffer> xPlatformCmdBufs;
	for (void* pCmdBuf : s_xCmdBuffersToSubmit) {
		vk::CommandBuffer& xBuf = *reinterpret_cast<vk::CommandBuffer*>(pCmdBuf);
		xPlatformCmdBufs.push_back(xBuf);
	}

	vk::SubmitInfo xSubmitInfo = vk::SubmitInfo()
		.setCommandBufferCount(s_xCmdBuffersToSubmit.size())
		.setPCommandBuffers(xPlatformCmdBufs.data())
		.setPWaitSemaphores(&pxRenderer->GetCurrentImageAvailableSem())
		.setPSignalSemaphores(&pxRenderer->GetCurrentRenderCompleteSem())
		.setWaitSemaphoreCount(1)
		.setSignalSemaphoreCount(1)
		.setWaitDstStageMask(eWaitStages);

	pxRenderer->GetGraphicsQueue().submit(xSubmitInfo, pxRenderer->GetCurrentInFlightFence());

	//TODO: put this in end frame when I eventually write it
	s_xCmdBuffersToSubmit.clear();
}