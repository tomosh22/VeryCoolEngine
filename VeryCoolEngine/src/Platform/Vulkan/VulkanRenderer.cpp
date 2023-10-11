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
	CreateInstance();
	InitDebugMessenger();
	GLFWwindow* pxWindow = (GLFWwindow*)Application::GetInstance()->GetWindow().GetNativeWindow();
	glfwCreateWindowSurface(m_instance, pxWindow, nullptr, (VkSurfaceKHR*)(&m_surface));
	SelectPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	
	CreateCommandPool();
	CreateDepthTexture();
	app->m_pxRenderPass = new VulkanRenderPass();
	CreateDescriptorPool();
	app->_pMesh->PlatformInit();
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	app->_shaders.back()->PlatformInit();
	app->_textures.back()->PlatformInit();
	//CreateGraphicsPipeline();


	m_xCameraLayout = VulkanDescriptorSetLayoutBuilder("Camera UBO")
		.WithUniformBuffers(1, vk::ShaderStageFlagBits::eVertex)
		.Build(m_device);
	m_xCameraDescriptor = CreateDescriptorSet(m_xCameraLayout, m_descriptorPool);

	m_xTextureLayout = VulkanDescriptorSetLayoutBuilder("Object Textures")
		.WithSamplers(1, vk::ShaderStageFlagBits::eFragment)
		.Build(m_device);
	m_xTextureDescriptor = CreateDescriptorSet(m_xTextureLayout, m_descriptorPool);

	UpdateImageDescriptor(m_xTextureDescriptor, 0, 0, dynamic_cast<VulkanTexture2D*>(app->_textures.back())->m_xImageView, dynamic_cast<VulkanTexture2D*>(app->_textures.back())->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		UpdateBufferDescriptor(m_xCameraDescriptor, dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO)->ppBuffers[i], 0, vk::DescriptorType::eUniformBuffer, 0);
	}
	

	//dynamic_cast<VulkanPipeline*>(app->m_pxPipeline)->PlatformInit();
	//app->m_pxPipeline
	
	app->m_pxGeometryPipeline = VulkanPipelineBuilder("Geometry Pipeline")
		.WithVertexInputState(dynamic_cast<VulkanMesh*>(app->_pMesh)->m_xVertexInputState)
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(*dynamic_cast<VulkanShader*>(app->_shaders.back()))
		.WithBlendState(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, false)
		.WithDepthState(vk::CompareOp::eGreaterOrEqual, true, true, false)
		.WithColourFormats({ vk::Format::eB8G8R8A8Srgb })
		.WithDepthFormat(vk::Format::eD32Sfloat)
		.WithDescriptorSetLayout(0, m_xCameraLayout)
		.WithDescriptorSetLayout(1, m_xTextureLayout)
		.WithPass(dynamic_cast<VulkanRenderPass*>(app->m_pxRenderPass)->m_xRenderPass)
		.Build();

	
	

	CreateFrameBuffers();
	
	CreateCommandBuffers();
	CreateSyncObjects();

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
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.pInheritanceInfo = nullptr;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit; //todo use prerecorded command buffer

	commandBuffer.begin(beginInfo);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxRenderPass)->m_xRenderPass;
	renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderPassInfo.renderArea.extent = m_swapChainExtent;

	vk::ClearValue clearColor[2];
	std::array<float, 4> tempColor{ 0.f,0.f,0.f,1.f };
	clearColor[0].color = {vk::ClearColorValue(tempColor)};
	clearColor[1].depthStencil = vk::ClearDepthStencilValue(0, 0);
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColor;

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, dynamic_cast<VulkanPipeline*>(app->m_pxGeometryPipeline)->m_xPipeline);

	vk::DescriptorSet aSets[] = {
		//dynamic_cast<VulkanPipeline*>(app->m_pxPipeline)->m_axDescriptorSets[m_currentFrame]
		m_xCameraDescriptor,
		m_xTextureDescriptor
	};

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, dynamic_cast<VulkanPipeline*>(app->m_pxGeometryPipeline)->m_xPipelineLayout, 0, sizeof(aSets) / sizeof(aSets[0]), aSets, 0, nullptr);

	//flipping because porting from opengl
	vk::Viewport viewport{};
	viewport.x = 0;
	viewport.y = m_swapChainExtent.height;
	viewport.width = m_swapChainExtent.width;
	viewport.height = -1 * (float)m_swapChainExtent.height;
	viewport.minDepth = 0;
	viewport.minDepth = 1;

	vk::Rect2D scissor{};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_swapChainExtent;

	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &scissor);

	for (Mesh* mesh : scene->meshes) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		vk::Buffer xVertexBuffer = pxVulkanMesh->m_pxVertexBuffer->m_pxVertexBuffer->m_xBuffer;
		vk::DeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, 1, &xVertexBuffer, offsets);

		vk::Buffer xIndexBuffer = pxVulkanMesh->m_pxIndexBuffer->m_pxIndexBuffer->m_xBuffer;
		commandBuffer.bindIndexBuffer(xIndexBuffer, 0, vk::IndexType::eUint32);

		if (pxVulkanMesh->m_pxInstanceBuffer != nullptr) {
			vk::Buffer xInstanceBuffer = pxVulkanMesh->m_pxInstanceBuffer->m_pxVertexBuffer->m_xBuffer;
			commandBuffer.bindVertexBuffers(1, 1, &xInstanceBuffer, offsets);
		}

		commandBuffer.drawIndexed(pxVulkanMesh->numIndices, app->_numInstances, 0, 0, 0);
		
	}
	


	commandBuffer.endRenderPass();
	commandBuffer.end();
}


void VulkanRenderer::DrawFrame(Scene* scene) {
#define UINT64_MAX std::numeric_limits<uint64_t>::max()
	m_device.waitForFences(1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);


	uint32_t imageIndex;
	vk::Result result = m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], nullptr, &imageIndex);

	if (result == vk::Result::eErrorOutOfDateKHR) {
		RecreateSwapChain();
		return;
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		std::cerr << "couldnt acquire swapchain image";

	m_device.resetFences(1, &m_inFlightFences[m_currentFrame]);
	m_commandBuffers[m_currentFrame].reset();
	RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex, scene);

	vk::SubmitInfo submitInfo{};

	vk::Semaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

	vk::Semaphore signalSempaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSempaphores;

	m_graphicsQueue.submit(submitInfo, m_inFlightFences[m_currentFrame]);

	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSempaphores;

	vk::SwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	m_presentQueue.presentKHR(&presentInfo);

	if (result == vk::Result::eErrorOutOfDateKHR || m_framebufferResized) {
		RecreateSwapChain();
		return;
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		std::cerr << "couldnt present swapchain image";

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
	app->_pMesh->PlatformInit();
	while (app->_running) {
		MainLoop();
	}

}


