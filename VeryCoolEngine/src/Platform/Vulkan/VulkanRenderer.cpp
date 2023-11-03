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


using namespace VeryCoolEngine;

VulkanRenderer* VulkanRenderer::s_pInstance = nullptr;

VulkanRenderer::VulkanRenderer() {
	InitWindow();
	InitVulkan();
}

void VulkanRenderer::InitWindow() {
	Application* app = Application::GetInstance();
	((WindowsWindow*)app->_window)->Init(WindowProperties());
}

void VulkanRenderer::InitVulkan() {
	VulkanRenderer::s_pInstance = this;
	Application* app = Application::GetInstance();

	BoilerplateInit();

#ifdef VCE_DEFERRED_SHADING
	SetupDeferredShading();
#endif
	
	for (Mesh* pMesh : app->_meshes) {
		pMesh->PlatformInit();
		pMesh->GetShader()->PlatformInit();
	}
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	app->_pLightUBO = ManagedUniformBuffer::Create(sizeof(Light) * _sMAXLIGHTS, MAX_FRAMES_IN_FLIGHT, 1);

	for (Shader* pxShader : app->_shaders) pxShader->PlatformInit();
	//for (Texture* pxTex : app->_textures) pxTex->PlatformInit();


	for (auto it = app->m_xPipelineSpecs.begin(); it != app->m_xPipelineSpecs.end(); it++) {
		m_xPipelines.emplace_back(VulkanPipelineBuilder::FromSpecification(it->second));
	}
	
	

	Application::GetInstance()->renderInitialised = true;
}

#ifdef VCE_DEFERRED_SHADING
void VulkanRenderer::SetupDeferredShading() {
	m_pxGBufferDiffuse = VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm);
	m_pxGBufferNormals = VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm);
	m_pxGBufferDepth = VulkanTexture2D::CreateDepthAttachment(m_width, m_height);
	m_pxDeferredDiffuse = VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm);
	m_pxDeferredSpecular = VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm);
}
#endif

void VulkanRenderer::MainLoop() {
	glfwPollEvents();

	Application* app = Application::GetInstance();

	Scene* scene = app->scene;
	while (true) {
		printf("Waiting on scene to be ready\n");
		if (scene->ready)break;//#todo implement mutex here
	}
	app->sceneMutex.lock();
	BeginScene(scene);

	

	DrawFrame(app->scene);

	


	app->sceneMutex.unlock();
}

void VulkanRenderer::RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {
	Application* app = Application::GetInstance();

	app->_pImGuiLayer->Begin();
	app->_pImGuiLayer->OnImGuiRender();
	
	

	commandBuffer.begin(vk::CommandBufferBeginInfo());

	BeginRenderPass(commandBuffer, imageIndex);


	for (VulkanPipeline*  pipeline : m_xPipelines) {

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->m_xPipeline);

		
		if (scene->m_axPipelineMeshes.find(pipeline->m_strName) == scene->m_axPipelineMeshes.end()) {
			continue;
		}
		for (Mesh* mesh : scene->m_axPipelineMeshes.at(pipeline->m_strName)) {
			VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
			pxVulkanMesh->BindToCmdBuffer(commandBuffer);

			std::vector<vk::DescriptorSet> axSets;
			for (const vk::DescriptorSet set : pipeline->m_axBufferDescSets)
				axSets.push_back(set);
			if (pxVulkanMesh->GetTexture() != nullptr)
				axSets.push_back(pxVulkanMesh->m_xTexDescSet);

			pipeline->BindDescriptorSets(commandBuffer, axSets, vk::PipelineBindPoint::eGraphics, 0);

			if (pipeline->bUsePushConstants) {
				commandBuffer.pushConstants(pipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, 0, sizeof(glm::mat4), (void*)&mesh->m_xTransform);

				commandBuffer.pushConstants(pipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4), sizeof(glm::vec3), (void*)&m_xOverrideNormal);
				int uValue = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;

				commandBuffer.pushConstants(pipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3), sizeof(uint32_t), (void*)&uValue);

				int uValueTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;

				commandBuffer.pushConstants(pipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t), sizeof(uint32_t), (void*)&uValueTess);

				commandBuffer.pushConstants(pipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(uint32_t), sizeof(float), (void*)&app->_pRenderer->m_fPhongTessFactor);
				commandBuffer.pushConstants(pipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(float), sizeof(float), (void*)&app->_pRenderer->m_uTessLevel);
			}

			commandBuffer.drawIndexed(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);
		}
	}



	


	
	
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	commandBuffer.endRenderPass();
	commandBuffer.end();

	app->_pImGuiLayer->End();
	
}

void VulkanRenderer::DrawFrame(Scene* scene) {
	uint32_t iImageIndex = AcquireSwapchainImage();
	if (iImageIndex == -1) {
		RecreateSwapChain();
		return;
	}

	RecordCommandBuffer(m_commandBuffers[m_currentFrame], iImageIndex, scene);

	SubmitCmdBuffer(m_commandBuffers[m_currentFrame], &m_imageAvailableSemaphores[m_currentFrame], 1, &m_renderFinishedSemaphores[m_currentFrame], 1, vk::PipelineStageFlagBits::eColorAttachmentOutput);

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
	app->_pLightUBO->UploadData(data, dataSize, 1, 0);
	delete[] data;
}

void VeryCoolEngine::VulkanRenderer::RenderThreadFunction()
{
	Application* app = Application::GetInstance();
	app->_pImGuiLayer = new ImGuiLayer();
	app->PushOverlay(app->_pImGuiLayer);
	while (app->_running) {
		MainLoop();
	}
}
