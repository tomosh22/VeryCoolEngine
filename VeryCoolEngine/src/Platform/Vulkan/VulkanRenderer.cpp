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
	
	for (Mesh* pMesh : app->_meshes) {
		pMesh->PlatformInit();
		pMesh->GetShader()->PlatformInit();
	}
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	for (Shader* pxShader : app->_shaders) pxShader->PlatformInit();
	for (Texture* pxTex : app->_textures) pxTex->PlatformInit();

	app->m_xCameraLayout = VulkanDescriptorSetLayoutBuilder("Camera UBO")
		.WithUniformBuffers(1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
		.Build(m_device);
	//m_xCameraDescriptor = CreateDescriptorSet(app->m_xCameraLayout, m_descriptorPool);

	app->m_xTextureLayout = VulkanDescriptorSetLayoutBuilder("Object Textures")
		.WithSamplers(1, vk::ShaderStageFlagBits::eFragment)
		.Build(m_device);


	app->m_xPipelines.emplace_back(VulkanPipelineBuilder::FromSpecification(app->m_xPipelineSpecs.at("Skybox")) );
	app->m_xPipelines.emplace_back(VulkanPipelineBuilder::FromSpecification(app->m_xPipelineSpecs.at("Blocks")));
	app->m_xPipelines.emplace_back(VulkanPipelineBuilder::FromSpecification(app->m_xPipelineSpecs.at("Terrain")));
	
	

	Application::GetInstance()->renderInitialised = true;
}

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


	for (VulkanPipeline*  pipeline : app->m_xPipelines) {

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->m_xPipeline);

		pipeline->BindDescriptorSets(commandBuffer, pipeline->m_axDescSets, vk::PipelineBindPoint::eGraphics, 0);

		for (Mesh* mesh : app->m_axPipelineMeshes.at(pipeline->m_strName)) {
			VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
			pxVulkanMesh->BindToCmdBuffer(commandBuffer);

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
