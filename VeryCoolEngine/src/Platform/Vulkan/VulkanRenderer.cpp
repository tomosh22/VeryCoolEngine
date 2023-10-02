#include "vcepch.h"
#include "VulkanRenderer.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Platform/Vulkan/VulkanMesh.h"

#include "Platform/Vulkan/VulkanManagedUniformBuffer.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanPipeline.h"




using namespace VeryCoolEngine;

VulkanRenderer* VulkanRenderer::s_pInstance = nullptr;

VulkanRenderer::VulkanRenderer() {
	InitWindow();
	InitVulkan();
}

void VulkanRenderer::InitWindow() {
	Application* app = Application::GetInstance();
	((WindowsWindow*)app->_window)->Init(WindowProperties());
#if 0
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_window = glfwCreateWindow(m_width, m_height, "VulkanTutorial", nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
#endif
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
	app->m_pxRenderPass = new VulkanRenderPass();
	CreateCommandPool();
	CreateDescriptorPool();
	app->_pMesh->PlatformInit();
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	app->_shaders.back()->PlatformInit();
	app->_textures.back()->PlatformInit();
	//CreateGraphicsPipeline();
	dynamic_cast<VulkanPipeline*>(app->m_pxPipeline)->PlatformInit();

	

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

void VulkanRenderer::Cleanup() {


	CleanupSwapChain();
	m_device.destroyDescriptorPool(m_descriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_device.destroySemaphore(m_imageAvailableSemaphores[i], nullptr);
		m_device.destroySemaphore(m_renderFinishedSemaphores[i], nullptr);
		m_device.destroyFence(m_inFlightFences[i], nullptr);
	}

	m_device.destroyCommandPool(m_commandPool, nullptr);
	m_device.destroy();

#if DEBUG
	m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr));
#endif


	m_instance.destroySurfaceKHR(m_surface, nullptr);
	m_instance.destroy();
	GLFWwindow* pxWindow = (GLFWwindow*)Application::GetInstance()->GetWindow().GetNativeWindow();
	glfwDestroyWindow(pxWindow);
	glfwTerminate();
}

void VulkanRenderer::CleanupSwapChain() {
	for (vk::Framebuffer framebuffer : m_swapChainFramebuffers) m_device.destroyFramebuffer(framebuffer, nullptr);

	for (vk::ImageView imageView : m_swapChainImageViews) m_device.destroyImageView(imageView, nullptr);

	m_device.destroySwapchainKHR(m_swapChain, nullptr);
}

void VulkanRenderer::CreateInstance() {
#if DEBUG
	if (!CheckValidationLayerSupport())throw std::runtime_error("validation layers requested, but not available!");
#endif
	vk::ApplicationInfo appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan Renderer")
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("No Engine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
		.setApiVersion(VK_API_VERSION_1_0);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#if DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	vk::InstanceCreateInfo instanceInfo = vk::InstanceCreateInfo()
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(extensions.size())
		.setPpEnabledExtensionNames(extensions.data())
#if DEBUG
		.setEnabledLayerCount(m_validationLayers.size())
		.setPpEnabledLayerNames(m_validationLayers.data());
#else
		.setEnabledLayerCount(0);
#endif
	m_instance = vk::createInstance(instanceInfo);
}

void VulkanRenderer::InitDebugMessenger() {
#if DEBUG
	vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		.setPfnUserCallback((PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback)
		.setPUserData(nullptr);
	m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(
		createInfo,
		nullptr,
		vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr)
	);
#endif

}

void VulkanRenderer::SelectPhysicalDevice() {
	uint32_t numDevices;
	m_instance.enumeratePhysicalDevices(&numDevices, nullptr);
	std::vector<vk::PhysicalDevice> devices;
	devices.resize(numDevices);
	m_instance.enumeratePhysicalDevices(&numDevices, devices.data());
	for (const vk::PhysicalDevice& device : devices) {
		if (CheckDeviceIsUsable(device)) {
			m_physicalDevice = device;
			break;
		}
	}
}

void VulkanRenderer::CreateLogicalDevice() {
	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

	std::vector<vk::DeviceQueueCreateInfo> queueInfos;
	std::set<uint32_t> uniqueFamilies = { indices.graphicsFamily, indices.presentFamily };
	float queuePriority = 1;
	for (uint32_t family : uniqueFamilies) {
		vk::DeviceQueueCreateInfo queueInfo = vk::DeviceQueueCreateInfo()
			.setQueueFamilyIndex(indices.graphicsFamily)
			.setQueueCount(1)
			.setPQueuePriorities(&queuePriority);
		queueInfos.push_back(queueInfo);

	}


	vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures()
		.setSamplerAnisotropy(VK_TRUE);

	vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
		.setPQueueCreateInfos(queueInfos.data())
		.setQueueCreateInfoCount(queueInfos.size())
		.setPEnabledFeatures(&deviceFeatures)
		.setEnabledExtensionCount(m_deviceExtensions.size())
		.setPpEnabledExtensionNames(m_deviceExtensions.data())
#if DEBUG
		.setEnabledLayerCount(m_validationLayers.size())
		.setPpEnabledLayerNames(m_validationLayers.data());
#else
		.setEnabledLayerCount(0);
#endif

	m_device = m_physicalDevice.createDevice(deviceCreateInfo);
	m_device.getQueue(indices.graphicsFamily, 0, &m_graphicsQueue);
	m_device.getQueue(indices.presentFamily, 0, &m_presentQueue);
}

QueueFamilyIndices VulkanRenderer::FindQueueFamilies(vk::PhysicalDevice physDevice) {
	QueueFamilyIndices indices;
	uint32_t numQueueFamilies = 0;
	physDevice.getQueueFamilyProperties(&numQueueFamilies, nullptr);

	std::vector<vk::QueueFamilyProperties> families;
	families.resize(numQueueFamilies);
	physDevice.getQueueFamilyProperties(&numQueueFamilies, families.data());
	int i = 0;
	for (const vk::QueueFamilyProperties& family : families) {
		if (family.queueFlags & vk::QueueFlagBits::eGraphics)indices.graphicsFamily = i;
		vk::Bool32 presentSupport = false;
		physDevice.getSurfaceSupportKHR(i, m_surface, &presentSupport);

		if (presentSupport)indices.presentFamily = i;

		if (indices.presentFamily != -1 && presentSupport)break;

		i++;
	}
	return indices;
}

SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(vk::PhysicalDevice physDevice) {
	SwapChainSupportDetails details;
	details.capabilities = physDevice.getSurfaceCapabilitiesKHR(m_surface);

	details.formats = physDevice.getSurfaceFormatsKHR(m_surface);

	details.presentModes = physDevice.getSurfacePresentModesKHR(m_surface);
	return details;
}

void VulkanRenderer::CreateSwapChain() {
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);
	vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;
	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
	uint32_t indicesPtr[] = { indices.graphicsFamily,indices.presentFamily };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indicesPtr;
	}
	else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	m_swapChain = m_device.createSwapchainKHR(createInfo);

	m_device.getSwapchainImagesKHR(m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	m_device.getSwapchainImagesKHR(m_swapChain, &imageCount, m_swapChainImages.data());

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

vk::SurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
	for (const vk::SurfaceFormatKHR& format : availableFormats) {
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)return format;
	}
	std::cerr << "b8g8r8a8_srgb not supported";
}

vk::Extent2D VulkanRenderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits <uint32_t>::max()) return capabilities.currentExtent;
	int extentWidth, extentHeight;
	GLFWwindow* pxWindow = (GLFWwindow*)Application::GetInstance()->GetWindow().GetNativeWindow();
	glfwGetFramebufferSize(pxWindow, &extentWidth, &extentHeight);
	vk::Extent2D extent = { (uint32_t)extentWidth,(uint32_t)extentHeight };
	extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	return extent;
}

void VulkanRenderer::CreateImageViews() {
	m_swapChainImageViews.resize(m_swapChainImages.size());
	for (int i = 0; i < m_swapChainImages.size(); i++)
	{
		vk::ImageViewCreateInfo createInfo{};
		createInfo.image = m_swapChainImages[i];
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = m_swapChainImageFormat;
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;
		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		m_swapChainImageViews[i] = m_device.createImageView(createInfo);
	}
}

vk::ShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code) {
	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	vk::ShaderModule module;
	module = m_device.createShaderModule(createInfo);
	return module;
}



void VulkanRenderer::CreateDescriptorPool() {
	
	vk::DescriptorPoolSize axPoolSizes[] =
	{
		{ vk::DescriptorType::eSampler, 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 },
		{ vk::DescriptorType::eSampledImage, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 },
		{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
		{ vk::DescriptorType::eInputAttachment, 1000 }
	};

	vk::DescriptorPoolCreateInfo xPoolInfo = vk::DescriptorPoolCreateInfo()
		.setPoolSizeCount(sizeof(axPoolSizes) / sizeof(axPoolSizes[0]))
		.setPPoolSizes(axPoolSizes)
		.setMaxSets(1000)
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	m_descriptorPool = m_device.createDescriptorPool(xPoolInfo);

}



vk::DescriptorSet VulkanRenderer::CreateDescriptorSet(const vk::DescriptorSetLayout& xLayout, const vk::DescriptorPool& xPool)
{
	vk::DescriptorSetAllocateInfo xInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(xPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&xLayout);

	return std::move(VulkanRenderer::GetInstance()->GetDevice().allocateDescriptorSets(xInfo)[0]);
}



void VulkanRenderer::CreateFrameBuffers() {
	Application* app = Application::GetInstance();
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
	int swapchainIndex = 0;
	for (vk::ImageView imageView : m_swapChainImageViews) {
		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxRenderPass)->m_xRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &imageView;
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;
		m_swapChainFramebuffers[swapchainIndex++] = m_device.createFramebuffer(framebufferInfo);
	}
}

void VulkanRenderer::CreateCommandPool() {
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	m_commandPool = m_device.createCommandPool(poolInfo);
}

void VulkanRenderer::CreateCommandBuffers() {
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
	m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
}

vk::CommandBuffer VulkanRenderer::BeginSingleUseCmdBuffer() {
	vk::CommandBufferAllocateInfo xAllocInfo = vk::CommandBufferAllocateInfo()
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(m_commandPool)
		.setCommandBufferCount(1);

	vk::CommandBuffer xCommandBuffer = m_device.allocateCommandBuffers(xAllocInfo)[0];

	vk::CommandBufferBeginInfo xBeginInfo = vk::CommandBufferBeginInfo{}
	.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	xCommandBuffer.begin(xBeginInfo);

	return xCommandBuffer;
}

void VulkanRenderer::EndSingleUseCmdBuffer(vk::CommandBuffer xBuffer) {
	xBuffer.end();

	vk::SubmitInfo xSubmitInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&xBuffer);

	m_graphicsQueue.submit(xSubmitInfo);
	m_graphicsQueue.waitIdle();

	m_device.freeCommandBuffers(m_commandPool, 1, &xBuffer);
}

void VulkanRenderer::ImageTransitionBarrier(vk::CommandBuffer xCmdBuffer, vk::Image xImage, vk::ImageLayout eOldLayout, vk::ImageLayout eNewLayout, vk::ImageAspectFlags eAspect, vk::PipelineStageFlags eSrcStage, vk::PipelineStageFlags eDstStage, int uMipLevel, int uLayer) {
	
	vk::ImageSubresourceRange xSubRange = vk::ImageSubresourceRange(eAspect, uMipLevel, 1, uLayer, 1);

	vk::ImageMemoryBarrier xMemoryBarrier = vk::ImageMemoryBarrier()
		.setSubresourceRange(xSubRange)
		.setImage(xImage)
		.setOldLayout(eOldLayout)
		.setNewLayout(eNewLayout);

	switch (eNewLayout) {
	case (vk::ImageLayout::eTransferDstOptimal):
		xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		break;
	case (vk::ImageLayout::eTransferSrcOptimal):
		xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		break;
	case (vk::ImageLayout::eColorAttachmentOptimal):
		xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		break;
	case (vk::ImageLayout::eDepthStencilAttachmentOptimal):
		xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		break;
	case (vk::ImageLayout::eShaderReadOnlyOptimal):
		xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead);
		break;
	default:
		VCE_ASSERT(false, "unkown layout");
		break;

		xCmdBuffer.pipelineBarrier(eSrcStage, eDstStage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &xMemoryBarrier);
	}
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

	vk::ClearValue clearColor;
	std::array<float, 4> temp{ 0.f,0.f,0.f,1.f };
	clearColor.color = { vk::ClearColorValue(temp) };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, dynamic_cast<VulkanPipeline*>(app->m_pxPipeline)->m_xPipeline);

	vk::DescriptorSet aSets[] = {
		dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO)->m_axDescriptorSets[m_currentFrame]
	};

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, dynamic_cast<VulkanPipeline*>(app->m_pxPipeline)->m_xPipelineLayout, 0, sizeof(aSets) / sizeof(aSets[0]), aSets, 0, nullptr);

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

void VulkanRenderer::CreateSyncObjects() {
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	vk::SemaphoreCreateInfo semaphoreInfo{};

	vk::FenceCreateInfo fenceInfo{};
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_imageAvailableSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
		m_renderFinishedSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
		m_inFlightFences[i] = m_device.createFence(fenceInfo);
	}

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

void VulkanRenderer::RecreateSwapChain() {
	int tempWidth = 0, tempHeight = 0;
	GLFWwindow* pxWindow = (GLFWwindow*)Application::GetInstance()->GetWindow().GetNativeWindow();
	glfwGetFramebufferSize(pxWindow, &tempWidth, &tempHeight);
	while (tempWidth == 0 || tempHeight == 0) {
		glfwGetFramebufferSize(pxWindow, &tempWidth, &tempHeight);
		glfwWaitEvents();
	}
	m_device.waitIdle();
	CleanupSwapChain();
	CreateSwapChain();
	CreateImageViews();
	CreateFrameBuffers();
}

bool VulkanRenderer::CheckValidationLayerSupport() {
	uint32_t numLayers;
	std::vector<vk::LayerProperties> availableLayers;
	vk::enumerateInstanceLayerProperties(&numLayers, nullptr);
	availableLayers.resize(numLayers);
	vk::enumerateInstanceLayerProperties(&numLayers, availableLayers.data());

	for (const char* layerName : m_validationLayers) {
		bool found = false;
		for (const vk::LayerProperties& layerProps : availableLayers) {
			if (std::string(layerName) == layerProps.layerName) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

void VulkanRenderer::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	VulkanRenderer* app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
	app->m_framebufferResized = true;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanRenderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';
	return VK_FALSE;
}

void VeryCoolEngine::VulkanRenderer::PlatformInit()
{
}

void VeryCoolEngine::VulkanRenderer::OnResize(uint32_t uWidth, uint32_t uHeight)
{
}

void VeryCoolEngine::VulkanRenderer::SetClearColor(const glm::vec4 color)
{
}

void VeryCoolEngine::VulkanRenderer::Clear()
{
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

void VeryCoolEngine::VulkanRenderer::EndScene()
{
}

void VeryCoolEngine::VulkanRenderer::BindViewProjMat(Shader* shader)
{
}

void VeryCoolEngine::VulkanRenderer::BindLightUBO(Shader* shader)
{
}

void VeryCoolEngine::VulkanRenderer::DrawFullScreenQuad()
{
}

void VeryCoolEngine::VulkanRenderer::RenderThreadFunction()
{
	Application* app = Application::GetInstance();
	app->_pMesh->PlatformInit();
	while (app->_running) {
		MainLoop();
	}

}

void VeryCoolEngine::VulkanRenderer::DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology)
{
}

void VeryCoolEngine::VulkanRenderer::DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology)
{
}
