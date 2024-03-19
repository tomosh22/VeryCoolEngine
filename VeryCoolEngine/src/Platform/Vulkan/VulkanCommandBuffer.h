#pragma once
#include "VeryCoolEngine/Renderer/RendererAPI.h"
namespace VeryCoolEngine {
	class VertexBuffer;
	class IndexBuffer;
	class Buffer;
	class Texture;

#define MAX_BINDINGS 16

	struct DescSetBindings {
		Buffer* m_xBuffers[MAX_BINDINGS];
		Texture* m_xTextures[MAX_BINDINGS];
	};

	enum BINDING_FREQUENCY : uint32_t {
		BINDING_FREQUENCY_PER_FRAME,
		BINDING_FREQUENCY_PER_DRAW,
		BINDING_FREQUENCY_MAX,
	};

	class VulkanCommandBuffer : public RendererAPI::CommandBuffer
	{
	public:
		VulkanCommandBuffer(bool bAsyncLoader);
		void BeginRecording() override;
		void EndRecording(RenderOrder eOrder, bool bEndPass = true) override;
		void SetVertexBuffer(VertexBuffer* xVertexBuffer, uint32_t uBindPoint = 0) override;
		void SetIndexBuffer(IndexBuffer* xIndexBuffer) override;
		void Draw(uint32_t uNumIndices, uint32_t uNumInstances = 1, uint32_t uVertexOffset = 0, uint32_t uIndexOffset = 0, uint32_t uInstanceOffset = 0) override;
		void SubmitTargetSetup(const RendererAPI::TargetSetup& xTargetSetup) override;
		void SetPipeline(void* pxPipeline) override;
		void BindTexture(void* pxTexture, uint32_t uBindPoint, uint32_t uSet) override;
		void BindBuffer(void* pxBuffer, uint32_t uBindPoint, uint32_t uSet) override;
		void PushConstant(void* pData, size_t uSize) override;
		void UploadUniformData(void* pData, size_t uSize) override;

		void BeginBind(BINDING_FREQUENCY eFreq);

		vk::CommandBuffer& GetCurrentCmdBuffer() { return m_xCurrentCmdBuffer; }
		void* Platform_GetCurrentCmdBuffer() const override { return (void*) & m_xCurrentCmdBuffer; }

		//currently unused
		vk::RenderPass TargetSetupToRenderPass(const RendererAPI::TargetSetup& xTargetSetup);
		//currently unused
		vk::Framebuffer TargetSetupToFramebuffer(const RendererAPI::TargetSetup& xTargetSetup);

		bool IsRecording() const override { return m_bIsRecording; }
	public:
		class VulkanRenderer* m_pxRenderer;

		std::vector<vk::CommandBuffer> m_xCmdBuffers;
		std::vector<vk::RenderPass> m_xRenderPasses;
		std::vector<vk::Framebuffer> m_xFramebuffers;

		vk::CommandBuffer m_xCurrentCmdBuffer;
		vk::RenderPass m_xCurrentRenderPass;
		vk::Framebuffer m_xCurrentFramebuffer;

		std::vector<vk::Semaphore> m_xCompleteSems;

		class VulkanPipeline* m_pxCurrentPipeline;

		DescSetBindings m_xBindings[BINDING_FREQUENCY_MAX];
		BINDING_FREQUENCY m_eCurrentBindFreq = BINDING_FREQUENCY_MAX;

		
	};

}