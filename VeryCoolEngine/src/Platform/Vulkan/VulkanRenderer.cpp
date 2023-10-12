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
	
	app->_pMesh->PlatformInit();
	app->m_pxQuadMesh->PlatformInit();
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	for (Shader* pxShader : app->_shaders) pxShader->PlatformInit();
	for (Texture2D* pxTex : app->_textures) pxTex->PlatformInit();


	m_xCameraLayout = VulkanDescriptorSetLayoutBuilder("Camera UBO")
		.WithUniformBuffers(1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
		.Build(m_device);
	m_xCameraDescriptor = CreateDescriptorSet(m_xCameraLayout, m_descriptorPool);

	m_xTextureLayout = VulkanDescriptorSetLayoutBuilder("Object Textures")
		.WithSamplers(1, vk::ShaderStageFlagBits::eFragment)
		.Build(m_device);
	m_xTextureDescriptor = CreateDescriptorSet(m_xTextureLayout, m_descriptorPool);

	/*m_xSkyboxTextureLayout = VulkanDescriptorSetLayoutBuilder("Skybox Texture")
		.WithSamplers(1, vk::ShaderStageFlagBits::eFragment)
		.Build(m_device);
	m_xSkyboxTextureDescriptor = CreateDescriptorSet(m_xTextureLayout, m_descriptorPool);*/

	UpdateImageDescriptor(m_xTextureDescriptor, 0, 0, dynamic_cast<VulkanTexture2D*>(app->_textures.at(0))->m_xImageView, dynamic_cast<VulkanTexture2D*>(app->_textures.at(0))->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		UpdateBufferDescriptor(m_xCameraDescriptor, dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO)->ppBuffers[i], 0, vk::DescriptorType::eUniformBuffer, 0);
	}
	
	
	app->m_pxGeometryPipeline = VulkanPipelineBuilder("Geometry Pipeline")
		.WithVertexInputState(dynamic_cast<VulkanMesh*>(app->_pMesh)->m_xVertexInputState)
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(*dynamic_cast<VulkanShader*>(app->_shaders.at(0)))
		.WithBlendState(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, false)
		.WithDepthState(vk::CompareOp::eGreaterOrEqual, true, true, false)
		.WithColourFormats({ vk::Format::eB8G8R8A8Srgb })
		.WithDepthFormat(vk::Format::eD32Sfloat)
		.WithDescriptorSetLayout(0, m_xCameraLayout)
		.WithDescriptorSetLayout(1, m_xTextureLayout)
		.WithPass(dynamic_cast<VulkanRenderPass*>(app->m_pxRenderPass)->m_xRenderPass)
		.Build();

	app->m_pxSkyboxPipeline = VulkanPipelineBuilder("Skybox Pipeline")
		.WithVertexInputState(dynamic_cast<VulkanMesh*>(app->m_pxQuadMesh)->m_xVertexInputState)
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(*dynamic_cast<VulkanShader*>(app->_shaders.at(1)))
		.WithBlendState(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, false)
		//.WithDepthState(vk::CompareOp::eGreaterOrEqual, true, true, false)
		.WithColourFormats({ vk::Format::eB8G8R8A8Srgb })
		//.WithDepthFormat(vk::Format::eD32Sfloat)
		.WithDescriptorSetLayout(0, m_xCameraLayout)
		//.WithDescriptorSetLayout(1, m_xTextureLayout)
		.WithPass(dynamic_cast<VulkanRenderPass*>(app->m_pxRenderPass)->m_xRenderPass)
		.Build();
	

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

	commandBuffer.begin(vk::CommandBufferBeginInfo());

	BeginRenderPass(commandBuffer, imageIndex);

	VulkanPipeline* pxSkyboxPipeline = dynamic_cast<VulkanPipeline*>(app->m_pxSkyboxPipeline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pxSkyboxPipeline->m_xPipeline);

	pxSkyboxPipeline->BindDescriptorSets(commandBuffer, { m_xCameraDescriptor }, vk::PipelineBindPoint::eGraphics, 0);

	VulkanMesh* pxSkyboxMesh = dynamic_cast<VulkanMesh*>(app->m_pxQuadMesh);
	pxSkyboxMesh->BindToCmdBuffer(commandBuffer);

	commandBuffer.drawIndexed(pxSkyboxMesh->m_uNumIndices, 1, 0, 0, 0);

	VulkanPipeline* pxGeometryPipeline = dynamic_cast<VulkanPipeline*>(app->m_pxGeometryPipeline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pxGeometryPipeline->m_xPipeline);

	pxGeometryPipeline->BindDescriptorSets(commandBuffer, { m_xCameraDescriptor, m_xTextureDescriptor }, vk::PipelineBindPoint::eGraphics, 0);


	for (Mesh* mesh : scene->meshes) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		pxVulkanMesh->BindToCmdBuffer(commandBuffer);

		commandBuffer.drawIndexed(pxVulkanMesh->m_uNumIndices, app->_numInstances, 0, 0, 0);
	}

	commandBuffer.endRenderPass();
	commandBuffer.end();
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
	while (app->_running) {
		MainLoop();
	}
}
