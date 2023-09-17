#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "VeryCoolEngine/Renderer/Renderer.h"
#include "VeryCoolEngine/Events/Event.h"

namespace VeryCoolEngine {
	class VulkanRenderer : public Renderer {
	public:
		~VulkanRenderer();

		void InitWindow() override;
		void PlatformInit() override;

		void SetClearColor(const glm::vec4 color) override;
		void Clear() override;
		void DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology = MeshTopolgy::Triangles) override;
		void DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology = MeshTopolgy::Triangles) override;
		void BindViewProjMat(Shader* shader) override;
		void BindLightUBO(Shader* shader) override;
		void DrawFullScreenQuad() override;
		void BeginScene(Scene* scene) override;
		void EndScene() override;
		void RenderThreadFunction() override;

		void OnEvent(Event& e);
	private:

		void RecordCmdBuffer(vk::CommandBuffer xCmd, uint32_t uImageIndex);

		void OnResize(uint32_t uWidth, uint32_t uHeight);

		void WaitForCmdBuffer(vk::CommandBuffer& buffer);

		std::vector<char> ReadFile(const char* szFilename);

		vk::ShaderModule CreateShaderModule(const std::vector<char>& xCode);

		vk::Instance m_xInstance;
		std::vector<const char*> m_xInstanceExtensions;
		std::vector<const char*> m_xInstanceLayers;
		vk::DebugUtilsMessengerEXT m_xDebugMessenger;

		vk::PhysicalDevice m_xPhysicalDevice;
		vk::Device m_xDevice;
		std::vector<const char*> m_xDeviceLayers;
		std::vector<const char*> m_xDeviceExtensions;

		vk::SurfaceKHR m_xSurface;
		vk::Format m_xSurfaceFormat;
		vk::ColorSpaceKHR m_xSurfaceColorSpace;

		std::vector<vk::QueueFamilyProperties> m_xQueueProps;
		uint32_t m_xGraphicsQueueIndex = -1;
		uint32_t m_xComputeQueueIndex = -1;
		uint32_t m_xCopyQueueIndex = -1;
		uint32_t m_xPresentQueueIndex = -1;

		vk::Queue m_xGraphicsQueue;
		vk::Queue m_xComputeQueue;
		vk::Queue m_xCopyQueue;
		vk::Queue m_xPresentQueue;

		vk::DescriptorPool m_xDefaultDescriptorPool;
		vk::CommandPool	m_xGraphicsCommandPool;	
		vk::CommandPool m_xComputeCommandPool;

		uint32_t m_uWidth;
		uint32_t m_uHeight;
		vk::Extent2D m_xSwapExtent;
		vk::Rect2D m_xScreenRect;
		vk::Viewport m_xViewport;
		vk::Rect2D m_xScissor;
		vk::SwapchainKHR m_xSwapchain;
		std::vector<vk::Image> m_xSwapchainImages;
		std::vector<vk::ImageView> m_xSwapchainImageViews;

		vk::ClearValue m_xClearColor;

		vk::RenderPass m_xRenderPass;
		vk::RenderPassBeginInfo m_xRenderPassBeginInfo;

		std::vector<vk::Framebuffer> m_xFramebuffers;

		vk::Semaphore m_xImageAvailableSem;
		vk::Semaphore m_xRenderFinishedSem;
		vk::Fence m_xInFlightFence;
		uint32_t m_uCurrentFrameIndex;

		vk::PipelineLayout m_xPipelineLayout;

		vk::Pipeline m_xPipeline;

		vk::CommandBuffer m_xCmdBuffer;
	};
}