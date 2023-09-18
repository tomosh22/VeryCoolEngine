#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
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

#define MAX_FRAMES_IN_FLIGHT 2
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
		protected:

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

			void CreateRenderPass();

			void CreateGraphicsPipeline();

			void CreateFrameBuffers();

			void CreateCommandPool();

			void CreateCommandBuffers();

			void RecordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

			void CreateSyncObjects();

			void DrawFrame();

			void RecreateSwapChain();



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

#if DEBUG
			bool CheckValidationLayerSupport();
#endif

			static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				vk::DebugUtilsMessageTypeFlagsEXT messageType,
				const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData);

			GLFWwindow* m_window;
			const int m_width = 1280, m_height = 720;
			vk::Instance m_instance;
			vk::DebugUtilsMessengerEXT m_debugMessenger;
			vk::PhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
			vk::Device m_device;
			vk::Queue m_graphicsQueue;
			vk::Queue m_presentQueue;
			vk::SurfaceKHR m_surface;
			vk::SwapchainKHR m_swapChain;
			std::vector<vk::Image> m_swapChainImages;
			vk::Format m_swapChainImageFormat;
			vk::Extent2D m_swapChainExtent;
			std::vector<vk::ImageView> m_swapChainImageViews;

			vk::RenderPass m_renderPass;
			vk::PipelineLayout m_pipelineLayout;
			vk::Pipeline m_graphicsPipeline;

			std::vector<vk::Framebuffer> m_swapChainFramebuffers;

			vk::CommandPool m_commandPool;
			std::vector<vk::CommandBuffer> m_commandBuffers;

			std::vector<vk::Semaphore> m_imageAvailableSemaphores;
			std::vector<vk::Semaphore> m_renderFinishedSemaphores;
			std::vector<vk::Fence> m_inFlightFences;

			uint32_t m_currentFrame = 0;

			bool m_framebufferResized = false;

			const std::vector<const char*> m_deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

#if DEBUG
			std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

#endif
		};
	



}