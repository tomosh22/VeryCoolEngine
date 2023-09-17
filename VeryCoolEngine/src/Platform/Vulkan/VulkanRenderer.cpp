#include "vcepch.h"
#include "VulkanRenderer.h"
#include "VeryCoolEngine/Application.h"
#include "Platform/Windows/WindowsWindow.h"

#include <fstream>


namespace VeryCoolEngine {

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VulkanRenderer::~VulkanRenderer() {
		m_xDevice.destroyPipelineLayout(m_xPipelineLayout);
		m_xInstance.destroy();

	}

	void VulkanRenderer::InitWindow()
	{
		Application* app = Application::GetInstance();
		glfwSetInputMode((GLFWwindow*)app->_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		((WindowsWindow*)app->_window)->Init(WindowProperties());


	}

	void VulkanRenderer::PlatformInit() {

		m_uCurrentFrameIndex = 0;

#pragma region Instance
		vk::ApplicationInfo xAppInfo = vk::ApplicationInfo()
			.setPApplicationName("CHANGE ME")
			.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
			.setPEngineName("VeryCoolEngine")
			.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
			.setApiVersion(VK_API_VERSION_1_0);

		bool bVulkanSupported = glfwVulkanSupported();

		uint32_t uNumGlfwExtensions;
		const char** szGlfwExtesnsions = glfwGetRequiredInstanceExtensions(&uNumGlfwExtensions);
		for (uint32_t i = 0; i < uNumGlfwExtensions; i++)
		{
			m_xInstanceExtensions.push_back(szGlfwExtesnsions[i]);
		}
		m_xInstanceExtensions.push_back("VK_EXT_debug_utils");

		m_xInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");

		vk::InstanceCreateInfo xInstanceInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&xAppInfo)
			.setEnabledExtensionCount(m_xInstanceExtensions.size())
			.setPpEnabledExtensionNames(m_xInstanceExtensions.data())
			.setEnabledLayerCount(m_xInstanceLayers.size())
			.setPpEnabledLayerNames(m_xInstanceLayers.data());

		m_xInstance = vk::createInstance(xInstanceInfo);

#pragma endregion

#pragma region DebugCallback
		vk::DebugUtilsMessageSeverityFlagsEXT xSeverityFlags =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

		vk::DebugUtilsMessageTypeFlagsEXT xMessageFlags =
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;


		vk::DebugUtilsMessengerCreateInfoEXT xDebugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
			.setMessageSeverity(xSeverityFlags)
			.setMessageType(xMessageFlags)
			.setPfnUserCallback(debugCallback);


		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_xInstance, "vkDestroyDebugUtilsMessengerEXT");
		func(m_xInstance, m_xDebugMessenger, nullptr);


#pragma endregion

#pragma region PhysicalDevice
		std::vector<vk::PhysicalDevice> xDevices = m_xInstance.enumeratePhysicalDevices();
#ifdef VCE_DEBUG
		bool bFoundDiscrete = false;
#endif
		for (vk::PhysicalDevice& device : xDevices) {
			if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				m_xPhysicalDevice = device;
#ifdef VCE_DEBUG
				bFoundDiscrete = true;
#endif
				break;
			}
		}
		VCE_ASSERT(bFoundDiscrete, "couldn't find discrete gpu");


#pragma endregion

#pragma region Surface
#ifdef VCE_WINDOWS
		vk::Win32SurfaceCreateInfoKHR xWindowCreateInfo = vk::Win32SurfaceCreateInfoKHR()
			.setHwnd(glfwGetWin32Window((GLFWwindow*)Application::GetInstance()->GetWindow().GetNativeWindow()))
			.setHinstance(GetModuleHandle(nullptr));
		m_xSurface = m_xInstance.createWin32SurfaceKHR(xWindowCreateInfo);
#endif
		std::vector<vk::SurfaceFormatKHR> xFormats = m_xPhysicalDevice.getSurfaceFormatsKHR(m_xSurface);
		m_xSurfaceFormat = xFormats[0].format;
		m_xSurfaceColorSpace = xFormats[0].colorSpace;

#pragma endregion



#pragma region QueueIndices
		m_xQueueProps = m_xPhysicalDevice.getQueueFamilyProperties();
		VkBool32 bSupportsPresent = false;
		for (uint32_t i = 0; i < m_xQueueProps.size(); i++) {
			bSupportsPresent = m_xPhysicalDevice.getSurfaceSupportKHR(i, m_xSurface);

			if (m_xComputeQueueIndex == -1 && m_xQueueProps[i].queueFlags & vk::QueueFlagBits::eCompute) {
				m_xComputeQueueIndex = i;
			}
			if (m_xCopyQueueIndex == -1 && m_xQueueProps[i].queueFlags & vk::QueueFlagBits::eTransfer) {
				m_xCopyQueueIndex = i;
			}

			if (m_xGraphicsQueueIndex == -1 && m_xQueueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
				m_xGraphicsQueueIndex = i;
				if (bSupportsPresent && m_xPresentQueueIndex == -1) {
					m_xPresentQueueIndex = i;
				}
			}
		}
		VCE_ASSERT(
			m_xGraphicsQueueIndex != -1 &&
			m_xComputeQueueIndex != -1 &&
			m_xCopyQueueIndex != -1 &&
			m_xPresentQueueIndex != -1
			, "missing queue indices"
		);
#pragma endregion

#pragma region LogicalDevice
		float queuePriority = 1;
		vk::DeviceQueueCreateInfo queueInfo = vk::DeviceQueueCreateInfo()
			.setQueueCount(1)
			.setQueueFamilyIndex(m_xGraphicsQueueIndex)
			.setPQueuePriorities(&queuePriority);

		vk::PhysicalDeviceFeatures features = vk::PhysicalDeviceFeatures();

		m_xDeviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		m_xDeviceExtensions.push_back("VK_KHR_swapchain");

		vk::DeviceCreateInfo xDeviceCreateInfo = vk::DeviceCreateInfo()
			.setQueueCreateInfoCount(1)
			.setPQueueCreateInfos(&queueInfo)
			.setEnabledLayerCount(m_xDeviceLayers.size())
			.setPpEnabledLayerNames(m_xDeviceLayers.data())
			.setEnabledExtensionCount(m_xDeviceExtensions.size())
			.setPpEnabledExtensionNames(m_xDeviceExtensions.data());

		m_xDevice = m_xPhysicalDevice.createDevice(xDeviceCreateInfo);

		m_xGraphicsQueue = m_xDevice.getQueue(m_xGraphicsQueueIndex, 0);
		m_xComputeQueue = m_xDevice.getQueue(m_xComputeQueueIndex, 0);
		m_xCopyQueue = m_xDevice.getQueue(m_xCopyQueueIndex, 0);
		m_xPresentQueue = m_xDevice.getQueue(m_xPresentQueueIndex, 0);
#pragma endregion

#pragma region Pools
		vk::CommandPoolCreateInfo xGraphicsPoolInfo = vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(m_xGraphicsQueueIndex);
		m_xGraphicsCommandPool = m_xDevice.createCommandPool(xGraphicsPoolInfo);

		vk::CommandPoolCreateInfo xComputePoolInfo = vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(m_xComputeQueueIndex);
		m_xComputeCommandPool = m_xDevice.createCommandPool(xComputePoolInfo);

		vk::DescriptorPoolSize pxPoolSizes[] ={
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

		vk::DescriptorPoolCreateInfo xPoolCreateInfo = vk::DescriptorPoolCreateInfo()
			.setMaxSets(1000)
			.setPoolSizeCount(sizeof(pxPoolSizes) / sizeof(pxPoolSizes[0]))
			.setPPoolSizes(pxPoolSizes)
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);


		m_xDefaultDescriptorPool = m_xDevice.createDescriptorPool(xPoolCreateInfo);

#pragma endregion

#pragma region CommandBuffers
		vk::CommandBufferAllocateInfo xCmdBufferAllocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(m_xGraphicsCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		m_xCmdBuffer = m_xDevice.allocateCommandBuffers(xCmdBufferAllocInfo)[0];
#pragma endregion



		OnResize(Application::GetInstance()->GetWindow().GetWidth(),
			Application::GetInstance()->GetWindow().GetHeight());

#pragma region SyncObjects

		vk::SemaphoreCreateInfo xSemaphoreInfo;
		vk::FenceCreateInfo xFenceInfo = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);

		
			m_xImageAvailableSem = m_xDevice.createSemaphore(xSemaphoreInfo);
			m_xRenderFinishedSem = m_xDevice.createSemaphore(xSemaphoreInfo);
			m_xInFlightFence = m_xDevice.createFence(xFenceInfo);
		
#pragma endregion

		Application* app = Application::GetInstance();
		app->_renderThreadCanStart = true;
		app->renderInitialised = true;
	}

	vk::ShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& xCode) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = xCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(xCode.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_xDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	std::vector<char> VulkanRenderer::ReadFile(const char* szFilename) {
		std::ifstream file(szFilename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void VulkanRenderer::SetClearColor(const glm::vec4 color)
	{
		std::array<float, 4> xArray = { (float)color.r, (float)color.g, (float)color.b, (float)color.a };
		m_xClearColor.color = vk::ClearColorValue(xArray);
	}

	void VulkanRenderer::Clear()
	{
	}

	void VulkanRenderer::DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology)
	{
	}

	void VulkanRenderer::DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology)
	{
	}

	void VulkanRenderer::BindViewProjMat(Shader* shader)
	{
	}

	void VulkanRenderer::BindLightUBO(Shader* shader)
	{
	}

	void VulkanRenderer::DrawFullScreenQuad()
	{
	}

	void VulkanRenderer::BeginScene(Scene* scene)
	{
	}

	void VulkanRenderer::EndScene()
	{
	}

	void VulkanRenderer::RenderThreadFunction()
	{
		Application* app = Application::GetInstance();
		while (app->_running) {

			m_xDevice.waitForFences(1, &m_xInFlightFence, VK_TRUE, UINT64_MAX);
			m_xDevice.resetFences(1, &m_xInFlightFence);

			m_uCurrentFrameIndex = m_xDevice.acquireNextImageKHR(m_xSwapchain, UINT64_MAX, m_xImageAvailableSem, vk::Fence()).value;

			m_xCmdBuffer.reset();

			RecordCmdBuffer(m_xCmdBuffer, m_uCurrentFrameIndex);
			vk::PipelineStageFlags eWaitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			vk::SubmitInfo xSubmitInfo = vk::SubmitInfo()
				.setWaitSemaphoreCount(1)
				.setPWaitSemaphores(&m_xImageAvailableSem)
				.setPWaitDstStageMask(&eWaitStage)
				.setCommandBufferCount(1)
				.setPCommandBuffers(&m_xCmdBuffer)
				.setSignalSemaphoreCount(1)
				.setPSignalSemaphores(&m_xRenderFinishedSem);

			m_xGraphicsQueue.submit(xSubmitInfo,m_xInFlightFence);

			vk::PresentInfoKHR xPresentInfo = vk::PresentInfoKHR()
				.setWaitSemaphoreCount(1)
				.setPWaitSemaphores(&m_xRenderFinishedSem)
				.setSwapchainCount(1)
				.setPSwapchains(&m_xSwapchain)
				.setPImageIndices(&m_uCurrentFrameIndex);
#ifdef VCE_DEBUG
			vk::Result xResult =
#endif
			m_xPresentQueue.presentKHR(xPresentInfo);

			VCE_ASSERT(xResult == vk::Result::eSuccess, "failed to present");
		}
	}

	void VulkanRenderer::OnEvent(Event& e)
	{
	}

	void VulkanRenderer::RecordCmdBuffer(vk::CommandBuffer xCmd, uint32_t uImageIndex)
	{
		xCmd.begin(vk::CommandBufferBeginInfo());

		vk::RenderPassBeginInfo xBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(m_xRenderPass)
			.setFramebuffer(m_xFramebuffers[uImageIndex])
			.setRenderArea({ { 0,0 }, m_xSwapExtent })
			.setClearValueCount(1)
			.setPClearValues(&m_xClearColor);

		xCmd.beginRenderPass(xBeginInfo, vk::SubpassContents::eInline);

		xCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_xPipeline);

		xCmd.setViewport(0, 1, &m_xViewport);
		xCmd.setScissor(0, 1, &m_xScissor);

		xCmd.draw(3, 1, 0, 0);

		xCmd.endRenderPass();

		xCmd.end();
	}

	void VulkanRenderer::OnResize(uint32_t uWidth, uint32_t uHeight){
		m_uWidth = uWidth;
		m_uHeight = uHeight;
		m_xScreenRect = vk::Rect2D( vk::Offset2D(0,0),vk::Extent2D(uWidth,uHeight));
		m_xScissor = vk::Rect2D( vk::Offset2D(0,0),vk::Extent2D(uWidth,uHeight));
		m_xViewport = vk::Viewport(0.0f, 0, (float)uWidth, (float)uHeight, 0.0f, 1.0f);

#pragma region Swapchain
		vk::SurfaceCapabilitiesKHR xSurfaceCapabilities = m_xPhysicalDevice.getSurfaceCapabilitiesKHR(m_xSurface);

		m_xSwapExtent = vk::Extent2D(
			Application::GetInstance()->GetWindow().GetWidth(),
			Application::GetInstance()->GetWindow().GetHeight());

		std::vector<vk::PresentModeKHR> xPresentModes = m_xPhysicalDevice.getSurfacePresentModesKHR(m_xSurface);

		vk::PresentModeKHR xPresentMode = vk::PresentModeKHR::eMailbox;

#ifdef VCE_DEBUG
		bool bMailBoxFound = false;
		for (vk::PresentModeKHR& mode : xPresentModes) {
			if (mode == vk::PresentModeKHR::eMailbox) {
				bMailBoxFound = true;
				break;
			}
		}
		VCE_ASSERT(bMailBoxFound, "what gpu are you using that doesn't support mailbox kekw");
#endif


		vk::SwapchainCreateInfoKHR xSwapInfo = vk::SwapchainCreateInfoKHR()
			.setPresentMode(xPresentMode)
			.setPreTransform(xSurfaceCapabilities.currentTransform)
			.setSurface(m_xSurface)
			.setImageColorSpace(m_xSurfaceColorSpace)
			.setImageFormat(m_xSurfaceFormat)
			.setImageExtent(m_xSwapExtent)
			.setMinImageCount(xSurfaceCapabilities.minImageCount + 1)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

		m_xSwapchain = m_xDevice.createSwapchainKHR(xSwapInfo);

		m_xSwapchainImages = m_xDevice.getSwapchainImagesKHR(m_xSwapchain);
		for (uint32_t i = 0; i < m_xSwapchainImages.size(); i++)
		{
			vk::ImageViewCreateInfo xImageViewInfo = vk::ImageViewCreateInfo()
				.setImage(m_xSwapchainImages[i])
				.setFormat(m_xSurfaceFormat)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
				.setViewType(vk::ImageViewType::e2D);
			m_xSwapchainImageViews.push_back(m_xDevice.createImageView(xImageViewInfo));
		}

#pragma endregion




#pragma region RenderPass
		if (m_xRenderPass) m_xDevice.destroyRenderPass(m_xRenderPass);

		//#todo depth buffer
		vk::AttachmentDescription xAttachDesc = vk::AttachmentDescription()
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
			.setFormat(m_xSurfaceFormat)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		vk::AttachmentReference xAttachRef = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription xSubpassDesc = vk::SubpassDescription()
			.setColorAttachmentCount(1)
			.setPColorAttachments(&xAttachRef)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		vk::SubpassDependency xDependInfo = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eNone)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo xRenderPassInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&xAttachDesc)
			.setSubpassCount(1)
			.setPSubpasses(&xSubpassDesc)
			.setDependencyCount(1)
			.setPDependencies(&xDependInfo);

		m_xRenderPass = m_xDevice.createRenderPass(xRenderPassInfo);
#pragma endregion

#pragma region Pipeline
		std::vector<const char*> xShaderFilenames = { "../Assets/Shaders/vulkan/vert.spv", "../Assets/Shaders/vulkan/frag.spv" };

		std::vector<char> szVertCode = ReadFile(xShaderFilenames[0]);
		std::vector<char> szFragCode = ReadFile(xShaderFilenames[1]);

		vk::ShaderModule xVertShaderModule = CreateShaderModule(szVertCode);
		vk::ShaderModule xFragShaderModule = CreateShaderModule(szFragCode);

		vk::PipelineShaderStageCreateInfo xVertStageInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(xVertShaderModule)
			.setPName("main");

		vk::PipelineShaderStageCreateInfo xFragStageInfo = vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(xFragShaderModule)
			.setPName("main");

		vk::PipelineVertexInputStateCreateInfo xVertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(0)
			.setPVertexBindingDescriptions(nullptr)
			.setVertexAttributeDescriptionCount(0)
			.setPVertexAttributeDescriptions(nullptr);

		vk::PipelineInputAssemblyStateCreateInfo xInputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

		vk::PipelineViewportStateCreateInfo xViewportInfo = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&m_xViewport)
			.setScissorCount(1)
			.setPScissors(&m_xScissor);

		vk::PipelineRasterizationStateCreateInfo xRasterInfo = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(VK_FALSE)
			.setRasterizerDiscardEnable(VK_FALSE)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1)
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthBiasConstantFactor(0)
			.setDepthBiasClamp(0)
			.setDepthBiasSlopeFactor(0);

		vk::PipelineMultisampleStateCreateInfo xMultisampleInfo = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(VK_FALSE)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(VK_FALSE)
			.setAlphaToOneEnable(VK_FALSE);

		vk::PipelineColorBlendAttachmentState xColorBlendAttachment = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA
			)
			.setBlendEnable(VK_FALSE)
			.setSrcColorBlendFactor(vk::BlendFactor::eOne)
			.setDstColorBlendFactor(vk::BlendFactor::eZero)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);

		vk::PipelineColorBlendStateCreateInfo xColorBlendInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachmentCount(1)
			.setPAttachments(&xColorBlendAttachment)
			.setBlendConstants({ 0,0,0,0 });

		vk::PipelineLayoutCreateInfo xLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(0)
			.setPSetLayouts(nullptr)
			.setPushConstantRangeCount(0)
			.setPPushConstantRanges(nullptr);

		m_xPipelineLayout = m_xDevice.createPipelineLayout(xLayoutCreateInfo);


		vk::PipelineShaderStageCreateInfo pShaderStages[2] = { xVertStageInfo, xFragStageInfo };

		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};
		vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStateCount(2)
			.setPDynamicStates(dynamicStates.data());

		vk::GraphicsPipelineCreateInfo xPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setStageCount(2)
			.setPStages(pShaderStages)
			.setPVertexInputState(&xVertexInputInfo)
			.setPInputAssemblyState(&xInputAssemblyInfo)
			.setPViewportState(&xViewportInfo)
			.setPRasterizationState(&xRasterInfo)
			.setPMultisampleState(&xMultisampleInfo)
			.setPDepthStencilState(nullptr)
			.setPColorBlendState(&xColorBlendInfo)
			.setPDynamicState(&dynamicState)
			.setLayout(m_xPipelineLayout)
			.setRenderPass(m_xRenderPass)
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(-1);

		m_xPipeline = m_xDevice.createGraphicsPipeline(VK_NULL_HANDLE, xPipelineCreateInfo).value;


		m_xDevice.destroyShaderModule(xVertShaderModule);
		m_xDevice.destroyShaderModule(xFragShaderModule);



#pragma endregion

#pragma region Framebuffers
		if (m_xFramebuffers.size()) {
			for (vk::Framebuffer& f : m_xFramebuffers) m_xDevice.destroyFramebuffer(f);
		}


		vk::FramebufferCreateInfo xFramebufferInfo = vk::FramebufferCreateInfo()
			.setWidth((uint32_t)Application::GetInstance()->GetWindow().GetWidth())
			.setHeight((uint32_t)Application::GetInstance()->GetWindow().GetHeight())
			.setRenderPass(m_xRenderPass)
			.setAttachmentCount(1)
			.setLayers(1);

		for (vk::ImageView& view : m_xSwapchainImageViews) {
			xFramebufferInfo.setPAttachments(&view);
			m_xFramebuffers.push_back(m_xDevice.createFramebuffer(xFramebufferInfo));
		}

		m_xRenderPassBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(m_xRenderPass)
			.setFramebuffer(m_xFramebuffers[m_uCurrentFrameIndex])
			.setRenderArea(m_xScissor)
			.setClearValueCount(1)
			.setPClearValues(&m_xClearColor);

		m_xDevice.waitIdle();


#pragma endregion


	}
	void VulkanRenderer::WaitForCmdBuffer(vk::CommandBuffer& buffer)
	{
		buffer.end();
		
		vk::Fence xFence = m_xDevice.createFence(vk::FenceCreateInfo());

		vk::SubmitInfo submitInfo = vk::SubmitInfo();
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&buffer);

		m_xGraphicsQueue.submit(submitInfo, xFence);

#ifdef VCE_DEBUG
		vk::Result eResult = 
#endif
		m_xDevice.waitForFences(1, &xFence, true, UINT64_MAX);

		VCE_ASSERT(eResult == vk::Result::eSuccess, "failed to wait on fence");
	}
}