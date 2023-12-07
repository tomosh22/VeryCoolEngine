#include "vcepch.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"

namespace VeryCoolEngine {
	VulkanCommandBuffer::VulkanCommandBuffer()
	{
		m_pxRenderer = VulkanRenderer::GetInstance();
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = m_pxRenderer->m_commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		m_xCmdBuffers = m_pxRenderer->m_device.allocateCommandBuffers(allocInfo);

		m_xCompleteSems.resize(MAX_FRAMES_IN_FLIGHT);
		vk::SemaphoreCreateInfo semaphoreInfo;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_xCompleteSems[i] = m_pxRenderer->m_device.createSemaphore(semaphoreInfo);
		}

		m_xRenderPasses.resize(MAX_FRAMES_IN_FLIGHT);
		m_xFramebuffers.resize(MAX_FRAMES_IN_FLIGHT);
	}
	void VulkanCommandBuffer::BeginRecording()
	{
		m_xCurrentCmdBuffer = m_xCmdBuffers[m_pxRenderer->m_currentFrame];
		m_xCurrentCmdBuffer.begin(vk::CommandBufferBeginInfo());
	}
	void VulkanCommandBuffer::EndRecording(bool bSubmit /*= true*/)
	{
		m_xCurrentCmdBuffer.end();
		if (bSubmit) {
			RendererAPI::s_xCmdBuffersToSubmit.push_back(&m_xCurrentCmdBuffer);
		}

	}
	void VulkanCommandBuffer::SetVertexBuffer(VertexBuffer* xVertexBuffer, uint32_t uBindPoint /*= 0*/)
	{
		VulkanVertexBuffer* pxVkVertexBuffer = dynamic_cast<VulkanVertexBuffer*>(xVertexBuffer);
		//TODO: offsets
		vk::DeviceSize offsets[] = { 0 };
		m_xCurrentCmdBuffer.bindVertexBuffers(uBindPoint, 1, &pxVkVertexBuffer->m_pxVertexBuffer->m_xBuffer, offsets);
	}
	void VulkanCommandBuffer::SetIndexBuffer(IndexBuffer* xIndexBuffer)
	{
		VulkanIndexBuffer* pxVkIndexBuffer = dynamic_cast<VulkanIndexBuffer*>(xIndexBuffer);
		m_xCurrentCmdBuffer.bindIndexBuffer(pxVkIndexBuffer->m_pxIndexBuffer->m_xBuffer, 0, vk::IndexType::eUint32);
	}
	void VulkanCommandBuffer::Draw(uint32_t uNumIndices, uint32_t uNumInstances /*= 1*/, uint32_t uVertexOffset /*= 0*/, uint32_t uIndexOffset /*= 0*/, uint32_t uInstanceOffset /*= 0*/)
	{
		m_xCurrentCmdBuffer.drawIndexed(uNumIndices, uNumInstances, uIndexOffset, uVertexOffset, uInstanceOffset);
	}

	vk::Format ConvertToVkFormat(RendererAPI::RenderTarget::Format eFormat) {
		switch (eFormat) {
		case RendererAPI::RenderTarget::Format::B8G8R8A8Srgb:
			return vk::Format::eB8G8R8A8Srgb;
		case RendererAPI::RenderTarget::Format::B8G8R8A8Unorm:
			return vk::Format::eB8G8R8A8Unorm;
		case RendererAPI::RenderTarget::Format::D32Sfloat:
			return vk::Format::eD32Sfloat;
		default:
			VCE_ASSERT(false, "Invalid format");
		}
	}

	vk::AttachmentLoadOp ConvertToVkLoadAction(RendererAPI::RenderTarget::LoadAction eAction) {
		switch (eAction) {
		case RendererAPI::RenderTarget::LoadAction::DontCare:
			return vk::AttachmentLoadOp::eDontCare;
		case RendererAPI::RenderTarget::LoadAction::Clear:
			return vk::AttachmentLoadOp::eClear;
		case RendererAPI::RenderTarget::LoadAction::Load:
			return vk::AttachmentLoadOp::eLoad;
		default:
			VCE_ASSERT(false, "Invalid action");
		}
	}

	vk::AttachmentStoreOp ConvertToVkStoreAction(RendererAPI::RenderTarget::StoreAction eAction) {
		switch (eAction) {
		case RendererAPI::RenderTarget::StoreAction::DontCare:
			return vk::AttachmentStoreOp::eDontCare;
		case RendererAPI::RenderTarget::StoreAction::Store:
			return vk::AttachmentStoreOp::eStore;
		default:
			VCE_ASSERT(false, "Invalid action");
		}
	}

	vk::RenderPass VulkanCommandBuffer::TargetSetupToRenderPass(const RendererAPI::TargetSetup& xTargetSetup) {
		const uint32_t uNumColourAttachments = xTargetSetup.m_xColourAttachments.size();
		std::vector<vk::AttachmentDescription> xAttachmentDescs(uNumColourAttachments);
		std::vector<vk::AttachmentReference> xAttachmentRefs(uNumColourAttachments);
		for (uint32_t i = 0; i < uNumColourAttachments; i++)
		{
			const RendererAPI::RenderTarget& xTarget = xTargetSetup.m_xColourAttachments.at(i);
			xAttachmentDescs.at(i)
				.setFormat(ConvertToVkFormat(xTarget.m_eFormat))
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(ConvertToVkLoadAction(xTarget.m_eLoadAction))
				.setStoreOp(ConvertToVkStoreAction(xTarget.m_eStoreAction))
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

			xAttachmentRefs.at(i)
				.setAttachment(i)
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		//should probably have a better way of checking for existence of depth/stencil target
		bool bHasDepth = xTargetSetup.m_xDepthStencil.m_eFormat != RendererAPI::RenderTarget::Format::None;
		vk::AttachmentDescription xDepthStencilAttachment;
		vk::AttachmentReference xDepthStencilAttachmentRef;
		if (bHasDepth) {
			xDepthStencilAttachment = vk::AttachmentDescription()
				.setFormat(ConvertToVkFormat(xTargetSetup.m_xDepthStencil.m_eFormat))
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(ConvertToVkLoadAction(xTargetSetup.m_xDepthStencil.m_eLoadAction))
				.setStoreOp(ConvertToVkStoreAction(xTargetSetup.m_xDepthStencil.m_eStoreAction))
				.setStencilLoadOp(ConvertToVkLoadAction(xTargetSetup.m_xDepthStencil.m_eLoadAction))
				.setStencilStoreOp(ConvertToVkStoreAction(xTargetSetup.m_xDepthStencil.m_eStoreAction))
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			xDepthStencilAttachmentRef = vk::AttachmentReference()
				.setAttachment(uNumColourAttachments)
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			xAttachmentDescs.push_back(xDepthStencilAttachment);
			xAttachmentRefs.push_back(xDepthStencilAttachmentRef);
		}

		vk::SubpassDescription xSubpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(uNumColourAttachments)
			.setPColorAttachments(xAttachmentRefs.data());
		if (bHasDepth)
			xSubpass.setPDepthStencilAttachment(&xDepthStencilAttachmentRef);

		vk::SubpassDependency xDependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits::eNone)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

		vk::RenderPassCreateInfo xRenderPassInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(xAttachmentDescs.size())
			.setPAttachments(xAttachmentDescs.data())
			.setSubpassCount(1)
			.setPSubpasses(&xSubpass)
			.setDependencyCount(1)
			.setPDependencies(&xDependency);

		return std::move(m_pxRenderer->GetDevice().createRenderPass(xRenderPassInfo));
	}

	vk::Framebuffer VulkanCommandBuffer::TargetSetupToFramebuffer(const RendererAPI::TargetSetup& xTargetSetup) {
		bool bHasDepth = xTargetSetup.m_xDepthStencil.m_eFormat != RendererAPI::RenderTarget::Format::None;
		const uint32_t uNumColourAttachments = xTargetSetup.m_xColourAttachments.size();
		const uint32_t uNumAttachments = bHasDepth ? uNumColourAttachments + 1 : uNumColourAttachments;

		vk::FramebufferCreateInfo framebufferInfo{};

		vk::ImageView* axAttachments = new vk::ImageView[uNumAttachments];
		for (uint32_t i = 0; i < uNumColourAttachments; i++) {
			axAttachments[i] = *reinterpret_cast<vk::ImageView*>(xTargetSetup.m_xColourAttachments[i].m_pPlatformImageView);
		}
		if(bHasDepth)
			axAttachments[uNumAttachments-1] = *reinterpret_cast<vk::ImageView*>(xTargetSetup.m_xDepthStencil.m_pPlatformImageView);

		framebufferInfo.renderPass = m_xCurrentRenderPass;
		framebufferInfo.attachmentCount = uNumAttachments;
		framebufferInfo.pAttachments = axAttachments;

		//assuming all targets are same resolution
		framebufferInfo.width = xTargetSetup.m_xColourAttachments[0].m_uWidth;
		framebufferInfo.height = xTargetSetup.m_xColourAttachments[0].m_uHeight;

		framebufferInfo.layers = 1;

		//is this how std::move works?
		vk::Framebuffer xFrameBuffer = std::move(m_pxRenderer->GetDevice().createFramebuffer(framebufferInfo));
		delete[] axAttachments;
		return xFrameBuffer;
	}

	void VulkanCommandBuffer::SubmitTargetSetup(const RendererAPI::TargetSetup& xTargetSetup)
	{
		uint32_t uIndexToDestroy = (m_pxRenderer->m_currentFrame + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT;
		//TODO: don't make these every single time
		m_pxRenderer->GetDevice().destroyRenderPass(m_xRenderPasses.at(uIndexToDestroy));
		m_xRenderPasses.at(m_pxRenderer->m_currentFrame) = TargetSetupToRenderPass(xTargetSetup);
		m_xCurrentRenderPass = m_xRenderPasses.at(m_pxRenderer->m_currentFrame);

		m_pxRenderer->GetDevice().destroyFramebuffer(m_xFramebuffers.at(uIndexToDestroy));
		m_xFramebuffers.at(m_pxRenderer->m_currentFrame) = TargetSetupToFramebuffer(xTargetSetup);
		m_xCurrentFramebuffer = m_xFramebuffers.at(m_pxRenderer->m_currentFrame);

		vk::RenderPassBeginInfo xRenderPassInfo = vk::RenderPassBeginInfo()
			.setRenderPass(m_xCurrentRenderPass)
			.setFramebuffer(m_xCurrentFramebuffer)
			.setRenderArea({ {0,0}, m_pxRenderer->m_swapChainExtent });

		vk::ClearValue* axClearColour = nullptr;
		//im being lazy and assuming all render targets have the same load action
		if (xTargetSetup.m_xColourAttachments[0].m_eLoadAction == RendererAPI::RenderTarget::LoadAction::Clear) {
			bool bHasDepth = xTargetSetup.m_xDepthStencil.m_eFormat != RendererAPI::RenderTarget::Format::None;
			const uint32_t uNumColourAttachments = xTargetSetup.m_xColourAttachments.size();
			const uint32_t uNumAttachments = bHasDepth ? uNumColourAttachments + 1 : uNumColourAttachments;
			axClearColour = new vk::ClearValue[uNumAttachments];
			std::array<float, 4> tempColour{ 0.f,0.f,0.f,1.f };
			for (uint32_t i = 0; i < uNumColourAttachments; i++)
			{
				axClearColour[i].color = { vk::ClearColorValue(tempColour) };
				axClearColour[i].depthStencil = vk::ClearDepthStencilValue(0, 0);
			}
			if(bHasDepth)
				axClearColour[uNumAttachments-1].depthStencil = vk::ClearDepthStencilValue(0, 0);

			xRenderPassInfo.clearValueCount = uNumAttachments;
			xRenderPassInfo.pClearValues = axClearColour;
		}


		m_xCurrentCmdBuffer.beginRenderPass(xRenderPassInfo, vk::SubpassContents::eInline);

		if (axClearColour != nullptr) delete[] axClearColour;

		//flipping because porting from opengl
		vk::Viewport xViewport{};
		xViewport.x = 0;
		xViewport.y = m_pxRenderer->m_swapChainExtent.height;
		xViewport.width = m_pxRenderer->m_swapChainExtent.width;
		xViewport.height = -1 * (float)m_pxRenderer->m_swapChainExtent.height;
		xViewport.minDepth = 0;
		xViewport.minDepth = 1;

		vk::Rect2D xScissor{};
		xScissor.offset = vk::Offset2D(0, 0);
		xScissor.extent = m_pxRenderer->m_swapChainExtent;

		m_xCurrentCmdBuffer.setViewport(0, 1, &xViewport);
		m_xCurrentCmdBuffer.setScissor(0, 1, &xScissor);
	}
	void VulkanCommandBuffer::SetPipeline(void* pxPipeline)
	{
		vk::Pipeline pxVkPipeline = *reinterpret_cast<vk::Pipeline*>(pxPipeline);
		m_xCurrentCmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pxVkPipeline);
	}
}


