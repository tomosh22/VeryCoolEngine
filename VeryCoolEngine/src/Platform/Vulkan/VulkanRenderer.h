#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

#include "VeryCoolEngine/Renderer/Renderer.h"
#include "VeryCoolEngine/Application.h"

#define DEBUG 1

namespace VeryCoolEngine {

	class VulkanMesh;
	class VulkanRenderPass;
	class VulkanTexture2D;
	class VulkanBuffer;
	

	static constexpr const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

		struct QueueFamilyIndices {
			uint32_t graphicsFamily = -1;
			uint32_t presentFamily = -1;
		};
		struct SwapChainSupportDetails {
			vk::SurfaceCapabilitiesKHR capabilities;
			std::vector <vk::SurfaceFormatKHR> formats;
			std::vector <vk::PresentModeKHR> presentModes;
		};
		class VulkanRenderer : public Renderer {
		public:
			
			friend class ImGuiLayer;
			friend class VulkanCommandBuffer;

			VulkanRenderer();

#pragma region overrides
			virtual void PlatformInit() override;
			virtual void OnResize(uint32_t uWidth, uint32_t uHeight) override;


			virtual void SetClearColor(const glm::vec4 color) override;
			virtual void Clear() override;

			virtual void BeginScene(Scene* scene) override;
			virtual void EndScene() override;

			virtual void BindViewProjMat(Shader* shader) override;
			virtual void BindLightUBO(Shader* shader) override;

			virtual void DrawFullScreenQuad() override;

			virtual void RenderThreadFunction() override;

			virtual void DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology = MeshTopolgy::Triangles) override;
			virtual void DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology = MeshTopolgy::Triangles) override;
#pragma endregion


			/*void run() {
				InitWindow();
				InitVulkan();
				MainLoop();
				Cleanup();
			}*/

			static VulkanRenderer* GetInstance() { return s_pInstance; }

			vk::Device& const GetDevice() { return m_device; }
			vk::PhysicalDevice& const GetPhysicalDevice() { return m_physicalDevice; }
			vk::CommandPool& const GetCommandPool() { return m_commandPool; }
			vk::Queue& const GetGraphicsQueue() { return m_graphicsQueue; }
			vk::DescriptorPool& const GetDescriptorPool() { return m_descriptorPool; }
			vk::Format& const GetSwapchainFormat() { return m_swapChainImageFormat; }

			vk::DescriptorSet CreateDescriptorSet(const vk::DescriptorSetLayout& xLayout, const vk::DescriptorPool& xPool);
			void UpdateBufferDescriptor(const vk::DescriptorSet& xSet, const VulkanBuffer* pxData, uint32_t uBinding, vk::DescriptorType eBufferType, size_t uOffset);
			void UpdateImageDescriptor(const vk::DescriptorSet& xSet, uint32_t uBinding, uint32_t uSubIndex, const vk::ImageView& xView, vk::Sampler& xSampler, vk::ImageLayout eLayout);

			vk::CommandBuffer BeginSingleUseCmdBuffer(vk::CommandBufferLevel eLevel = vk::CommandBufferLevel::ePrimary);
			void EndSingleUseCmdBuffer(vk::CommandBuffer xBuffer);

			void ImageTransitionBarrier(vk::Image xImage, vk::ImageLayout eOldLayout, vk::ImageLayout eNewLayout, vk::ImageAspectFlags eAspect, vk::PipelineStageFlags eSrcStage, vk::PipelineStageFlags eDstStage, int uMipLevel = 0, int uLayer = 0);

			static std::vector<char> ReadFile(const std::string& filename) {
				std::ifstream file(filename, std::ios::ate | std::ios::binary);
				if (!file.is_open())std::cerr << "couldnt open file";
				int fileSize = file.tellg();
				std::vector<char> buffer(fileSize);
				file.seekg(0);
				file.read(buffer.data(), fileSize);
				file.close();
				return buffer;
			}

			const vk::Semaphore& GetCurrentImageAvailableSem() const { return m_imageAvailableSemaphores[m_currentFrame];}
			const vk::Semaphore& GetCurrentRenderCompleteSem() const { return m_renderFinishedSemaphores[m_currentFrame];}
			const vk::Fence& GetCurrentInFlightFence() const { return m_inFlightFences[m_currentFrame];}
			const vk::Queue& GetGraphicsQueue() const { return m_graphicsQueue; }

			//TODO: move to private
			uint32_t m_currentFrame = 0;

		protected:
			static VulkanRenderer* s_pInstance;

			void InitWindow() override;
			void InitVulkan();
			void MainLoop() override;
			void Cleanup();

			void CleanupSwapChain();

			void CreateInstance();



			void InitDebugMessenger();

			void SelectPhysicalDevice();

			void CreateLogicalDevice();

			bool CheckDeviceIsUsable(vk::PhysicalDevice physDevice) {
				//vk::PhysicalDeviceProperties props;
				//physDevice.getProperties(&props);
				//vk::PhysicalDeviceFeatures features;
				//physDevice.getFeatures(features);
				//
				//QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physDevice);
				//if (!(props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
				//	queueFamilyIndices.graphicsFamily != -1 &&
				//	queueFamilyIndices.presentFamily != -1
				//	&& CheckDeviceExtensionsSupport(physDevice))) {
				//	std::cerr << "incompatible gpu\n";
				//	return false;
				//}
				//SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physDevice);
				//return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
				return true;

			}
			bool CheckDeviceExtensionsSupport(vk::PhysicalDevice physDevice) {
				//uint32_t numExtensions = 0;
				//
				//physDevice.enumerateDeviceExtensionProperties(nullptr, &numExtensions, nullptr);
				//std::vector<vk::ExtensionProperties> availableExtensions;
				//availableExtensions.resize(numExtensions);
				//physDevice.enumerateDeviceExtensionProperties(nullptr, &numExtensions, availableExtensions.data());
				//std::set<std::string> requiredExtensions;
				//for (const char* ext : deviceExtensions) {
				//	requiredExtensions.insert(ext);
				//}
				//for (const vk::ExtensionProperties& ext : availableExtensions) {
				//	requiredExtensions.erase(ext.extensionName);
				//}
				//return requiredExtensions.empty();
				return true;
			}

			QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice physDevice);

			SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice physDevice);

			void CreateSwapChain();

			vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

			vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
				for (const vk::PresentModeKHR& mode : availablePresentModes) {
					if (mode == vk::PresentModeKHR::eMailbox)return mode;
				}
				std::cerr << "mailbox not supported";
			}

			vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

			void CreateImageViews();

			vk::ShaderModule CreateShaderModule(const std::vector<char>& code);


			void CreateDescriptorPool();

			void CreateDepthTexture();


			void CreateFrameBuffers();
			void CreateImguiFrameBuffers();//imgui doesn't use depth buffer
			void CreateGBufferFrameBuffers();
			void CreateRenderToTextureFrameBuffers();

			void CreateCommandPool();

			void CreateCommandBuffers();

			void BoilerplateInit();

			

			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);

			void CreateSyncObjects();

			void DrawFrame(Scene* scene);

			void RecreateSwapChain();

			int8_t AcquireSwapchainImage();

			void SubmitCmdBuffer(vk::CommandBuffer& xCmdBuffer, vk::Semaphore* pxWaitSems = nullptr, uint32_t uWaitSemCount = 0, vk::Semaphore* pxSignalSems = nullptr, uint32_t uSignalSemCount = 0, vk::PipelineStageFlags eWaitStages = vk::PipelineStageFlagBits::eNone);

			void Present(uint32_t uSwapchainIndex, vk::Semaphore* pxWaitSems = nullptr, uint32_t uWaitSemCount = 0);

			void BeginBackbufferRenderPass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex);
			void BeginGBufferRenderPass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex);
			void BeginImguiRenderPass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex);//imgui doesn't use depth
			void BeginRenderToTexturePass(vk::CommandBuffer& xCmdBuffer, uint32_t uImageIndex);

#if DEBUG
			bool CheckValidationLayerSupport();
#endif

			static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				vk::DebugUtilsMessageTypeFlagsEXT messageType,
				const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData);

#ifdef VCE_USE_EDITOR
			int m_width = VCE_GAME_WIDTH + VCE_EDITOR_ADDITIONAL_WIDTH, m_height = VCE_GAME_HEIGHT + VCE_EDITOR_ADDITIONAL_HEIGHT;
#else
			int m_width = VCE_GAME_WIDTH, m_height = VCE_GAME_HEIGHT;
#endif
			vk::Instance m_instance;
			vk::DebugUtilsMessengerEXT m_debugMessenger;
			
			vk::Device m_device;
			vk::PhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

			
			vk::Queue m_graphicsQueue;
			vk::Queue m_presentQueue;
			vk::SurfaceKHR m_surface;
			vk::SwapchainKHR m_swapChain;
			std::vector<vk::Image> m_swapChainImages;
			vk::Format m_swapChainImageFormat;
			vk::Extent2D m_swapChainExtent;
			std::vector<vk::ImageView> m_swapChainImageViews;


			std::vector<class VulkanPipeline*> m_xPipelines;

			RendererAPI::TargetSetup CreateGBufferTarget();

#ifdef VCE_DEFERRED_SHADING
			void SetupDeferredShading();

			//one per frame in flight
			std::vector<VulkanTexture2D*> m_apxGBufferDiffuse;
			std::vector<VulkanTexture2D*> m_apxGBufferNormals;
			std::vector<VulkanTexture2D*> m_apxGBufferMaterial;
			std::vector<VulkanTexture2D*> m_apxGBufferDepth;
			std::vector<VulkanTexture2D*> m_apxDeferredDiffuse;
			std::vector<VulkanTexture2D*> m_apxDeferredSpecular;

			std::vector<VulkanTexture2D*> m_apxEditorSceneTexs;
#endif

			vk::DescriptorPool m_descriptorPool;

			/*vk::DescriptorSetLayout m_xCameraLayout;
			vk::DescriptorSetLayout m_xTextureLayou
			vk::DescriptorSetLayout m_xSkyboxTextureLayout;
			vk::DescriptorSet m_xCameraDescriptor;
			vk::DescriptorSet m_xTextureDescriptor;*/
			vk::DescriptorSet m_xSkyboxTextureDescriptor;

			VulkanTexture2D* m_xDepthTexture;

			vk::Sampler m_xDefaultSampler;
			std::vector<vk::DescriptorSet> m_axFramebufferTexDescSet;

			vk::DescriptorSetLayout m_descriptorLayout;
			vk::PipelineLayout m_pipelineLayout;

			std::vector<vk::Framebuffer> m_swapChainFramebuffers;
			std::vector<vk::Framebuffer> m_axImguiFramebuffers;
			std::vector<vk::Framebuffer> m_axGBufferFramebuffers;
			std::vector<vk::Framebuffer> m_axRenderToTextureFramebuffers;

			vk::CommandPool m_commandPool;
			std::vector<vk::CommandBuffer> m_commandBuffers;
			class VulkanCommandBuffer* m_pxCommandBuffer;

			std::vector<vk::Semaphore> m_imageAvailableSemaphores;
			std::vector<vk::Semaphore> m_renderFinishedSemaphores;
			std::vector<vk::Fence> m_inFlightFences;

			

			bool m_framebufferResized = false;

			const std::vector<const char*> m_deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

#if DEBUG
			std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

#endif

		};
	



}