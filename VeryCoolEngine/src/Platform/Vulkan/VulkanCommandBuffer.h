#pragma once
#include "VeryCoolEngine/Renderer/RendererAPI.h"
namespace VeryCoolEngine {
	class VertexBuffer;
	class IndexBuffer;
	class VulkanCommandBuffer : RendererAPI::CommandBuffer
	{
	public:
		VulkanCommandBuffer();
		void BeginRecording() override;
		void EndRecording(bool bSubmit = true) override;
		void SetVertexBuffer(VertexBuffer* xVertexBuffer, uint32_t uBindPoint = 0) override;
		void SetIndexBuffer(IndexBuffer* xIndexBuffer) override;
		void Draw(uint32_t uNumIndices, uint32_t uNumInstances = 1, uint32_t uVertexOffset = 0, uint32_t uIndexOffset = 0, uint32_t uInstanceOffset = 0) override;
		void SubmitTargetSetup(const RendererAPI::TargetSetup& xTargetSetup) override;
		void SetPipeline(void* pxPipeline) override;
		void BindTexture(void* pxTexture, uint32_t uBindPoint) override;

		vk::CommandBuffer& GetCurrentCmdBuffer() { return m_xCurrentCmdBuffer; }
		void* Platform_GetCurrentCmdBuffer() const override { return (void*) & m_xCurrentCmdBuffer; }

		vk::RenderPass TargetSetupToRenderPass(const RendererAPI::TargetSetup& xTargetSetup);
		vk::Framebuffer TargetSetupToFramebuffer(const RendererAPI::TargetSetup& xTargetSetup);
	private:
		class VulkanRenderer* m_pxRenderer;

		std::vector<vk::CommandBuffer> m_xCmdBuffers;
		std::vector<vk::RenderPass> m_xRenderPasses;
		std::vector<vk::Framebuffer> m_xFramebuffers;

		vk::CommandBuffer m_xCurrentCmdBuffer;
		vk::RenderPass m_xCurrentRenderPass;
		vk::Framebuffer m_xCurrentFramebuffer;

		std::vector<vk::Semaphore> m_xCompleteSems;

		class VulkanPipeline* m_pxCurrentPipeline;

		uint32_t m_uCurrentDescSetIndex = 0;
	};

}