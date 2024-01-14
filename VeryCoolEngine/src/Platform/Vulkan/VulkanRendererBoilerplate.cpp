#include "vcepch.h"
#include "VulkanRenderer.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanTexture.h"
namespace VeryCoolEngine {

	void VulkanRenderer::BoilerplateInit() {
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
		CreateDescriptorPool();

		//#TODO do this properly, I'm too lazy to move the above to a VulkanRenderPass::BackbufferRenderPass
		app->m_pxGBufferRenderPass = new VulkanRenderPass();
		m_device.destroyRenderPass(dynamic_cast<VulkanRenderPass*>(app->m_pxGBufferRenderPass)->m_xRenderPass);
		dynamic_cast<VulkanRenderPass*>(app->m_pxGBufferRenderPass)->m_xRenderPass = VulkanRenderPass::GBufferRenderPass();

		app->m_pxImguiRenderPass = new VulkanRenderPass();
		m_device.destroyRenderPass(dynamic_cast<VulkanRenderPass*>(app->m_pxImguiRenderPass)->m_xRenderPass);
		dynamic_cast<VulkanRenderPass*>(app->m_pxImguiRenderPass)->m_xRenderPass = VulkanRenderPass::ImguiRenderPass();

		app->m_pxRenderToTexturePass = new VulkanRenderPass();
		m_device.destroyRenderPass(dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePass)->m_xRenderPass);
		dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePass)->m_xRenderPass = VulkanRenderPass::RenderToTexturePass();

		app->m_pxRenderToTexturePassNoClear = new VulkanRenderPass();
		m_device.destroyRenderPass(dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePassNoClear)->m_xRenderPass);
		dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePassNoClear)->m_xRenderPass = VulkanRenderPass::RenderToTexturePassNoClear();

		app->m_pxCopyToFramebufferPass = new VulkanRenderPass();
		m_device.destroyRenderPass(dynamic_cast<VulkanRenderPass*>(app->m_pxCopyToFramebufferPass)->m_xRenderPass);
		dynamic_cast<VulkanRenderPass*>(app->m_pxCopyToFramebufferPass)->m_xRenderPass = VulkanRenderPass::CopyToFramebufferPass();

		for (uint32_t i = 0; i < m_swapChainImages.size(); i++) {
			m_apxEditorSceneTexs.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Srgb));
		}

		CreateFrameBuffers();
#ifdef VCE_USE_EDITOR
		CreateImguiFrameBuffers();//imgui doesn't use depth
#endif
		CreateRenderToTextureFrameBuffers();
		CreateRenderToTextureFrameBuffersNoClear();

#ifdef VCE_DEFERRED_SHADING
		SetupDeferredShading();
		CreateGBufferFrameBuffers();
#endif
		
		CreateCommandBuffers();
		CreateSyncObjects();


		m_xDefaultSampler = VulkanTexture2D::CreateSampler();

		app->m_pxBlankTexture2D->PlatformInit();
		
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

		delete m_xDepthTexture;

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
		QueueFamilyIndices m_puIndices = FindQueueFamilies(m_physicalDevice);

		std::vector<vk::DeviceQueueCreateInfo> queueInfos;
		std::set<uint32_t> uniqueFamilies = { m_puIndices.graphicsFamily, m_puIndices.presentFamily };
		float queuePriority = 1;
		for (uint32_t family : uniqueFamilies) {
			vk::DeviceQueueCreateInfo queueInfo = vk::DeviceQueueCreateInfo()
				.setQueueFamilyIndex(m_puIndices.graphicsFamily)
				.setQueueCount(1)
				.setPQueuePriorities(&queuePriority);
			queueInfos.push_back(queueInfo);

		}


		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures()
			.setSamplerAnisotropy(VK_TRUE)
			.setTessellationShader(VK_TRUE);

		vk::PhysicalDeviceFeatures2 deviceFeatures2;
		deviceFeatures2.setFeatures(deviceFeatures);

		vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures;
		indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;
		indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
		indexingFeatures.descriptorBindingPartiallyBound = true;
		indexingFeatures.descriptorBindingVariableDescriptorCount = true;
		indexingFeatures.runtimeDescriptorArray = true;
		deviceFeatures2.setPNext((void*)&indexingFeatures);

		vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
			.setPNext(&deviceFeatures2)
			.setPQueueCreateInfos(queueInfos.data())
			.setQueueCreateInfoCount(queueInfos.size())
			.setEnabledExtensionCount(m_deviceExtensions.size())
			.setPpEnabledExtensionNames(m_deviceExtensions.data())
#if DEBUG
			.setEnabledLayerCount(m_validationLayers.size())
			.setPpEnabledLayerNames(m_validationLayers.data());
#else
			.setEnabledLayerCount(0);
#endif

		m_device = m_physicalDevice.createDevice(deviceCreateInfo);
		m_device.getQueue(m_puIndices.graphicsFamily, 0, &m_graphicsQueue);
		m_device.getQueue(m_puIndices.presentFamily, 0, &m_presentQueue);
	}

	QueueFamilyIndices VulkanRenderer::FindQueueFamilies(vk::PhysicalDevice physDevice) {
		QueueFamilyIndices m_puIndices;
		uint32_t numQueueFamilies = 0;
		physDevice.getQueueFamilyProperties(&numQueueFamilies, nullptr);

		std::vector<vk::QueueFamilyProperties> families;
		families.resize(numQueueFamilies);
		physDevice.getQueueFamilyProperties(&numQueueFamilies, families.data());
		int i = 0;
		for (const vk::QueueFamilyProperties& family : families) {
			if (family.queueFlags & vk::QueueFlagBits::eGraphics)m_puIndices.graphicsFamily = i;
			vk::Bool32 presentSupport = false;
			physDevice.getSurfaceSupportKHR(i, m_surface, &presentSupport);

			if (presentSupport)m_puIndices.presentFamily = i;

			if (m_puIndices.presentFamily != -1 && presentSupport)break;

			i++;
		}
		return m_puIndices;
	}

	SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(vk::PhysicalDevice physDevice) {
		SwapChainSupportDetails details;
		details.capabilities = physDevice.getSurfaceCapabilitiesKHR(m_surface);

		details.formats = physDevice.getSurfaceFormatsKHR(m_surface);

		details.presentModes = physDevice.getSurfacePresentModesKHR(m_surface);
		return details;
	}

	void VulkanRenderer::CreateFrameBuffers() {
		Application* app = Application::GetInstance();
		m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
		int swapchainIndex = 0;
		for (vk::ImageView imageView : m_swapChainImageViews) {
			vk::FramebufferCreateInfo framebufferInfo{};
			vk::ImageView axAttachments[]{
				imageView,
			};
			framebufferInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxCopyToFramebufferPass)->m_xRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = axAttachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;
			m_swapChainFramebuffers[swapchainIndex++] = m_device.createFramebuffer(framebufferInfo);


			TextureDescriptorSpecification xTexSpec;
			xTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
			xTexSpec.m_bJustFragment = true;
			xTexSpec.m_bBindless = false;
			vk::DescriptorSetLayout xLayout = VulkanDescriptorSetLayoutBuilder::FromSpecification(xTexSpec);
			m_axFramebufferTexDescSet.emplace_back(CreateDescriptorSet(xLayout, m_descriptorPool));

		}
	}

	void VulkanRenderer::CreateRenderToTextureFrameBuffers() {
		Application* app = Application::GetInstance();
		m_axRenderToTextureFramebuffers.resize(m_swapChainImageViews.size());
		int swapchainIndex = 0;
		for (vk::ImageView imageView : m_swapChainImageViews) {
			vk::FramebufferCreateInfo framebufferInfo{};
			vk::ImageView axAttachments[]{
				m_apxEditorSceneTexs[swapchainIndex]->m_xImageView,
				m_xDepthTexture->m_xImageView,
			};
			framebufferInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePass)->m_xRenderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = axAttachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;
			m_axRenderToTextureFramebuffers[swapchainIndex++] = m_device.createFramebuffer(framebufferInfo);

		}
	}

	void VulkanRenderer::CreateRenderToTextureFrameBuffersNoClear() {
		Application* app = Application::GetInstance();
		m_axRenderToTextureFramebuffersNoClear.resize(m_swapChainImageViews.size());
		int swapchainIndex = 0;
		for (vk::ImageView imageView : m_swapChainImageViews) {
			vk::FramebufferCreateInfo framebufferInfo{};
			vk::ImageView axAttachments[]{
				m_apxEditorSceneTexs[swapchainIndex]->m_xImageView,
				m_xDepthTexture->m_xImageView,
			};
			framebufferInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePassNoClear)->m_xRenderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = axAttachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;
			m_axRenderToTextureFramebuffersNoClear[swapchainIndex++] = m_device.createFramebuffer(framebufferInfo);

		}
	}

	void VulkanRenderer::CreateImguiFrameBuffers() {
		Application* app = Application::GetInstance();
		m_axImguiFramebuffers.resize(m_swapChainImageViews.size());
		int swapchainIndex = 0;
		for (vk::ImageView imageView : m_swapChainImageViews) {
			vk::FramebufferCreateInfo framebufferInfo{};
			vk::ImageView axAttachments[1]{ imageView };
			framebufferInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxImguiRenderPass)->m_xRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = axAttachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;
			m_axImguiFramebuffers[swapchainIndex++] = m_device.createFramebuffer(framebufferInfo);
		}
	}
#ifdef VCE_DEFERRED_SHADING
	void VulkanRenderer::CreateGBufferFrameBuffers() {
		Application* app = Application::GetInstance();
		m_axGBufferFramebuffers.resize(m_swapChainImageViews.size());
		for (uint32_t i = 0; i < m_swapChainImages.size(); i++) {
			vk::FramebufferCreateInfo framebufferInfo{};
			vk::ImageView axAttachments[4]{ m_apxGBufferDiffuse[i]->m_xImageView, m_apxGBufferNormals[i]->m_xImageView,m_apxGBufferMaterial[i]->m_xImageView, m_apxGBufferDepth[i]->m_xImageView };
			framebufferInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxGBufferRenderPass)->m_xRenderPass;
			framebufferInfo.attachmentCount = 4;
			framebufferInfo.pAttachments = axAttachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;
			m_axGBufferFramebuffers[i] = m_device.createFramebuffer(framebufferInfo);
		}
	}


	RendererAPI::TargetSetup VulkanRenderer::CreateGBufferTarget() {
		Application* app = Application::GetInstance();
		RendererAPI::TargetSetup xTargetSetup;
		RendererAPI::RenderTarget xDiffuseTarget;
		RendererAPI::RenderTarget xNormalTarget;
		RendererAPI::RenderTarget xMaterialTarget;
		RendererAPI::RenderTarget xDepthStencilTarget;

		xDiffuseTarget.m_eFormat = RendererAPI::RenderTarget::Format::B8G8R8A8Unorm;
		xDiffuseTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xDiffuseTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xDiffuseTarget.m_uHeight = m_height;
		xDiffuseTarget.m_uWidth = m_width;
		xDiffuseTarget.m_pPlatformImageView = &m_apxGBufferDiffuse[m_currentFrame]->m_xImageView;

		xNormalTarget.m_eFormat = RendererAPI::RenderTarget::Format::B8G8R8A8Unorm;
		xNormalTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xNormalTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xNormalTarget.m_uHeight = m_height;
		xNormalTarget.m_uWidth = m_width;
		xNormalTarget.m_pPlatformImageView = &m_apxGBufferNormals[m_currentFrame]->m_xImageView;

		xMaterialTarget.m_eFormat = RendererAPI::RenderTarget::Format::B8G8R8A8Unorm;
		xMaterialTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xMaterialTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xMaterialTarget.m_uHeight = m_height;
		xMaterialTarget.m_uWidth = m_width;
		xMaterialTarget.m_pPlatformImageView = &m_apxGBufferMaterial[m_currentFrame]->m_xImageView;

		xDepthStencilTarget.m_eFormat = RendererAPI::RenderTarget::Format::D32Sfloat;
		xDepthStencilTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xDepthStencilTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xDepthStencilTarget.m_uHeight = m_height;
		xDepthStencilTarget.m_uWidth = m_width;
		xDepthStencilTarget.m_pPlatformImageView = &m_apxGBufferDepth[m_currentFrame]->m_xImageView;

		xTargetSetup.m_xColourAttachments.push_back(xDiffuseTarget);
		xTargetSetup.m_xColourAttachments.push_back(xNormalTarget);
		xTargetSetup.m_xColourAttachments.push_back(xMaterialTarget);
		xTargetSetup.m_xDepthStencil = xDepthStencilTarget;

		return xTargetSetup;
	}
#endif


	RendererAPI::TargetSetup VulkanRenderer::CreateFramebufferTarget() {
		Application* app = Application::GetInstance();
		RendererAPI::TargetSetup xTargetSetup;
		RendererAPI::RenderTarget xColourTarget;
		RendererAPI::RenderTarget xDepthStencilTarget;

		xColourTarget.m_eFormat = RendererAPI::RenderTarget::Format::B8G8R8A8Srgb;
		xColourTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xColourTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xColourTarget.m_uHeight = m_height;
		xColourTarget.m_uWidth = m_width;
		
		xColourTarget.m_eUsage = RendererAPI::RenderTarget::Usage::Present;

		xDepthStencilTarget.m_eFormat = RendererAPI::RenderTarget::Format::D32Sfloat;
		xDepthStencilTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xDepthStencilTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xDepthStencilTarget.m_uHeight = m_height;
		xDepthStencilTarget.m_uWidth = m_width;
		xDepthStencilTarget.m_pPlatformImageView = &m_xDepthTexture->m_xImageView;

		xTargetSetup.m_xColourAttachments.push_back(xColourTarget);
		xTargetSetup.m_xDepthStencil = xDepthStencilTarget;

		xTargetSetup.m_strName = "CopyToFramebuffer";

		m_xTargetSetupPasses.insert({ "CopyToFramebuffer" ,dynamic_cast<VulkanRenderPass*>(app->m_pxCopyToFramebufferPass)->m_xRenderPass });

		m_xTargetSetupFramebuffers.insert({ "CopyToFramebuffer" ,m_swapChainFramebuffers });

		return xTargetSetup;
	}

	

	RendererAPI::TargetSetup VulkanRenderer::CreateRenderToTextureTarget() {
		Application* app = Application::GetInstance();
		RendererAPI::TargetSetup xTargetSetup;
		RendererAPI::RenderTarget xColourTarget;
		RendererAPI::RenderTarget xDepthStencilTarget;

		xColourTarget.m_eFormat = RendererAPI::RenderTarget::Format::B8G8R8A8Srgb;
		xColourTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xColourTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xColourTarget.m_uHeight = m_height;
		xColourTarget.m_uWidth = m_width;

		xColourTarget.m_eUsage = RendererAPI::RenderTarget::Usage::ShaderRead;

		xDepthStencilTarget.m_eFormat = RendererAPI::RenderTarget::Format::D32Sfloat;
		xDepthStencilTarget.m_eLoadAction = RendererAPI::RenderTarget::LoadAction::Clear;
		xDepthStencilTarget.m_eStoreAction = RendererAPI::RenderTarget::StoreAction::Store;
		xDepthStencilTarget.m_uHeight = m_height;
		xDepthStencilTarget.m_uWidth = m_width;
		xDepthStencilTarget.m_pPlatformImageView = &m_xDepthTexture->m_xImageView;

		xTargetSetup.m_xColourAttachments.push_back(xColourTarget);
		xTargetSetup.m_xDepthStencil = xDepthStencilTarget;

		xTargetSetup.m_strName = "RenderToTexture";

		m_xTargetSetupPasses.insert({ "RenderToTexture" ,dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePass)->m_xRenderPass });
		m_xTargetSetupPasses.insert({ "RenderToTextureNoClear" ,dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePassNoClear)->m_xRenderPass });

		m_xTargetSetupFramebuffers.insert({ "RenderToTexture" ,m_axRenderToTextureFramebuffers });
		m_xTargetSetupFramebuffers.insert({ "RenderToTextureNoClear" ,m_axRenderToTextureFramebuffersNoClear });

		return xTargetSetup;
	}

	void VulkanRenderer::CreateSwapChain() {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);
		vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		//do i need to + 1 here?
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount;

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

		QueueFamilyIndices m_puIndices = FindQueueFamilies(m_physicalDevice);
		uint32_t indicesPtr[] = { m_puIndices.graphicsFamily,m_puIndices.presentFamily };
		if (m_puIndices.graphicsFamily != m_puIndices.presentFamily) {
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
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);

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

	void VulkanRenderer::UpdateBufferDescriptor(const vk::DescriptorSet& xSet, const VulkanBuffer* pxData, uint32_t uBinding, vk::DescriptorType eBufferType, size_t uOffset /* = 0*/) {
		vk::DescriptorBufferInfo xInfo = vk::DescriptorBufferInfo()
			.setBuffer(pxData->m_xBuffer)
			.setOffset(uOffset)
			.setRange(pxData->GetSize());

		vk::WriteDescriptorSet xWrite = vk::WriteDescriptorSet()
			.setDescriptorType(eBufferType)
			.setDstSet(xSet)
			.setDstBinding(uBinding)
			.setDescriptorCount(1)
			.setPBufferInfo(&xInfo);

		m_device.updateDescriptorSets(1, &xWrite, 0, nullptr);
	}

	void VulkanRenderer::UpdateImageDescriptor(const vk::DescriptorSet& xSet, uint32_t uBinding, uint32_t uSubIndex, const vk::ImageView& xView, vk::Sampler& xSampler, vk::ImageLayout eLayout) {
		vk::DescriptorImageInfo xInfo = vk::DescriptorImageInfo()
			.setSampler(xSampler)
			.setImageView(xView)
			.setImageLayout(eLayout);

		vk::WriteDescriptorSet xWrite = vk::WriteDescriptorSet()
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDstSet(xSet)
			.setDstBinding(uBinding)
			.setDstArrayElement(uSubIndex)
			.setDescriptorCount(1)
			.setPImageInfo(&xInfo);

		m_device.updateDescriptorSets(1, &xWrite, 0, nullptr);
	}

	void VulkanRenderer::CreateCommandPool() {
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		m_commandPool = m_device.createCommandPool(poolInfo);
	}

	void VulkanRenderer::CreateDepthTexture() {
		vk::FormatProperties xProps = m_physicalDevice.getFormatProperties(vk::Format::eD32Sfloat);



		uint32_t uWidth = Application::GetInstance()->GetWindow().GetWidth();
		uint32_t uHeight = Application::GetInstance()->GetWindow().GetHeight();

		m_xDepthTexture = dynamic_cast<VulkanTexture2D*>(Texture2D::Create(uWidth, uHeight, TextureFormat::D));
		m_xDepthTexture->m_bIsDepthTexture = true;
		m_xDepthTexture->PlatformInit();
	}

	void VulkanRenderer::CreateCommandBuffers() {
		m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
		m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
	}

	vk::CommandBuffer VulkanRenderer::BeginSingleUseCmdBuffer(vk::CommandBufferLevel eLevel) {
		vk::CommandBufferAllocateInfo xAllocInfo = vk::CommandBufferAllocateInfo()
			.setLevel(eLevel)
			.setCommandPool(m_commandPool)
			.setCommandBufferCount(1);

		vk::CommandBuffer xCommandBuffer = m_device.allocateCommandBuffers(xAllocInfo)[0];

		vk::CommandBufferBeginInfo xBeginInfo = vk::CommandBufferBeginInfo{}
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		if (eLevel == vk::CommandBufferLevel::eSecondary) {
			xBeginInfo.setFlags(xBeginInfo.flags | vk::CommandBufferUsageFlagBits::eRenderPassContinue);

			vk::CommandBufferInheritanceInfo xInherit = vk::CommandBufferInheritanceInfo()
				.setRenderPass(dynamic_cast<VulkanRenderPass*>(Application::GetInstance()->m_pxCopyToFramebufferPass)->m_xRenderPass)
				.setFramebuffer(m_swapChainFramebuffers[m_currentFrame]);
			xBeginInfo.setPInheritanceInfo(&xInherit);
		}

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

	void VulkanRenderer::ImageTransitionBarrier(vk::Image xImage, vk::ImageLayout eOldLayout, vk::ImageLayout eNewLayout, vk::ImageAspectFlags eAspect, vk::PipelineStageFlags eSrcStage, vk::PipelineStageFlags eDstStage, int uMipLevel, int uLayer) {
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::CommandBuffer xCmd = pxRenderer->BeginSingleUseCmdBuffer();
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
		case (vk::ImageLayout::eDepthAttachmentOptimal):
			xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
			break;
		case (vk::ImageLayout::eDepthStencilAttachmentOptimal):
			xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
			break;
		case (vk::ImageLayout::eShaderReadOnlyOptimal):
			xMemoryBarrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead);
			break;
		default:
			VCE_ASSERT(false, "unknown layout");
			break;
		}

		xCmd.pipelineBarrier(eSrcStage, eDstStage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &xMemoryBarrier);

		pxRenderer->EndSingleUseCmdBuffer(xCmd);
	}

	void VulkanRenderer::CreateSyncObjects() {
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_xSkyboxRenderedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		vk::SemaphoreCreateInfo semaphoreInfo{};

		vk::FenceCreateInfo fenceInfo{};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_imageAvailableSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
			m_renderFinishedSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
			m_xSkyboxRenderedSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
			m_inFlightFences[i] = m_device.createFence(fenceInfo);
		}

	}

	void VeryCoolEngine::VulkanRenderer::DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology)
	{
	}

	void VeryCoolEngine::VulkanRenderer::DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology)
	{
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

	void VulkanRenderer::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		VulkanRenderer* app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
		app->m_framebufferResized = true;
	}



	VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanRenderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';
		}
		__debugbreak();
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
		CreateDepthTexture();
		CreateImageViews();
		CreateFrameBuffers();
	}

	int8_t VulkanRenderer::AcquireSwapchainImage() {
		m_device.waitForFences(1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);


		uint32_t uImageIndex;
		vk::Result result = m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], nullptr, &uImageIndex);

		if (result == vk::Result::eErrorOutOfDateKHR) return -1;
		VCE_ASSERT(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR, "Failed to acquire swapchain image");

		m_device.resetFences(1, &m_inFlightFences[m_currentFrame]);

		return static_cast<int8_t>(uImageIndex);
	}

	void VulkanRenderer::SubmitCmdBuffers(const std::vector<vk::CommandBuffer>& xCmdBuffers, const std::vector<vk::Semaphore>& xWaitSems, const std::vector<vk::Semaphore>& xSignalSems, vk::Fence xFence, vk::PipelineStageFlags eWaitStages) {

		vk::SubmitInfo xSubmitInfo = vk::SubmitInfo()
			.setCommandBufferCount(xCmdBuffers.size())
			.setPCommandBuffers(xCmdBuffers.data())
			.setPWaitSemaphores(xWaitSems.data())
			.setPSignalSemaphores(xSignalSems.data())
			.setWaitSemaphoreCount(xWaitSems.size())
			.setSignalSemaphoreCount(xSignalSems.size())
			.setWaitDstStageMask(eWaitStages);


		VCE_ASSERT(xSubmitInfo.waitSemaphoreCount == xWaitSems.size() && xSubmitInfo.signalSemaphoreCount == xSignalSems.size(), "Something wrong with wait stages?");

		m_graphicsQueue.submit(xSubmitInfo, xFence);
	}

	void VulkanRenderer::Present(uint32_t uSwapchainIndex, vk::Semaphore* pxWaitSems, uint32_t uWaitSemCount) {
		vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
			.setSwapchainCount(1)
			.setPSwapchains(&m_swapChain)
			.setPImageIndices(&uSwapchainIndex);

		if (uWaitSemCount > 0) {
			presentInfo.setWaitSemaphoreCount(uWaitSemCount);
			presentInfo.setPWaitSemaphores(pxWaitSems);
		}
#ifdef VCE_DEBUG
		vk::Result xResult =
#endif
			m_presentQueue.presentKHR(&presentInfo);

		VCE_ASSERT(xResult == vk::Result::eSuccess || xResult == vk::Result::eErrorOutOfDateKHR || xResult == vk::Result::eSuboptimalKHR, "Failed to present");
	}

	void VulkanRenderer::BeginBackbufferRenderPass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex) {
		Application* app = Application::GetInstance();
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = m_xTargetSetupPasses.at("CopyToFramebuffer");
		renderPassInfo.framebuffer = m_xTargetSetupFramebuffers.at("CopyToFramebuffer").at(m_currentFrame);
		renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
		renderPassInfo.renderArea.extent = m_swapChainExtent;


		vk::ClearValue clearColor[2];
		std::array<float, 4> tempColor{ 0.f,0.f,0.f,1.f };
		clearColor[0].color = { vk::ClearColorValue(tempColor) };
		clearColor[1].depthStencil = vk::ClearDepthStencilValue(0, 0);
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearColor;
		

		xCmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

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

		xCmdBuffer.setViewport(0, 1, &viewport);
		xCmdBuffer.setScissor(0, 1, &scissor);
	}

	void VulkanRenderer::BeginGBufferRenderPass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex) {
		Application* app = Application::GetInstance();
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxGBufferRenderPass)->m_xRenderPass;
		renderPassInfo.framebuffer = m_axGBufferFramebuffers[uImageIndex];
		renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
		renderPassInfo.renderArea.extent = m_swapChainExtent;

		vk::ClearValue clearColor[4];
		std::array<float, 4> tempColor{ 0.f,0.f,0.f,1.f };
		clearColor[0].color = { vk::ClearColorValue(tempColor) };
		clearColor[1].color = { vk::ClearColorValue(tempColor) };
		clearColor[2].color = { vk::ClearColorValue(tempColor) };
		clearColor[3].depthStencil = vk::ClearDepthStencilValue(0, 0);
		renderPassInfo.clearValueCount = 4;
		renderPassInfo.pClearValues = clearColor;

		xCmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

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

		xCmdBuffer.setViewport(0, 1, &viewport);
		xCmdBuffer.setScissor(0, 1, &scissor);
	}

	void VulkanRenderer::BeginImguiRenderPass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex) {
		Application* app = Application::GetInstance();
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxImguiRenderPass)->m_xRenderPass;
		renderPassInfo.framebuffer = m_swapChainFramebuffers[uImageIndex];
		renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
		renderPassInfo.renderArea.extent = m_swapChainExtent;


		xCmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

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

		xCmdBuffer.setViewport(0, 1, &viewport);
		xCmdBuffer.setScissor(0, 1, &scissor);
	}

	void VulkanRenderer::BeginRenderToTexturePass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex) {
		Application* app = Application::GetInstance();
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePass)->m_xRenderPass;
		renderPassInfo.framebuffer = m_axRenderToTextureFramebuffers[uImageIndex];
		renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
		renderPassInfo.renderArea.extent = m_swapChainExtent;

		vk::ClearValue clearColor[2];
		std::array<float, 4> tempColor{ 0.f,0.f,0.f,1.f };
		clearColor[0].color = { vk::ClearColorValue(tempColor) };
		clearColor[1].depthStencil = vk::ClearDepthStencilValue(0, 0);
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearColor;

		xCmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

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

		xCmdBuffer.setViewport(0, 1, &viewport);
		xCmdBuffer.setScissor(0, 1, &scissor);
	}
}