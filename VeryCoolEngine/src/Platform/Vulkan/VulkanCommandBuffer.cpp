#include "vcepch.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanPipelineBuilder.h"
#include "VulkanManagedUniformBuffer.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "VulkanRenderPass.h"

namespace VeryCoolEngine {
	VulkanCommandBuffer::VulkanCommandBuffer(bool bAsyncLoader)
	{
		m_pxRenderer = VulkanRenderer::GetInstance();
		vk::CommandBufferAllocateInfo allocInfo{};
		if(bAsyncLoader)
			allocInfo.commandPool = m_pxRenderer->m_xAsyncLoaderCommandPool;
		else
			allocInfo.commandPool = m_pxRenderer->m_commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		m_xCmdBuffers = m_pxRenderer->m_device.allocateCommandBuffers(allocInfo);

		m_xCompleteSems.resize(MAX_FRAMES_IN_FLIGHT);
		vk::SemaphoreCreateInfo semaphoreInfo;
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_xCompleteSems[i] = m_pxRenderer->m_device.createSemaphore(semaphoreInfo);
		}

		m_xRenderPasses.resize(MAX_FRAMES_IN_FLIGHT);
		m_xFramebuffers.resize(MAX_FRAMES_IN_FLIGHT);

		m_pxUniformBuffer = new VulkanManagedUniformBuffer;
	}
	void VulkanCommandBuffer::BeginRecording()
	{
		m_xCurrentCmdBuffer = m_xCmdBuffers[m_pxRenderer->m_currentFrame];
		m_xCurrentCmdBuffer.begin(vk::CommandBufferBeginInfo());

		m_eCurrentBindFreq = RendererAPI::BINDING_FREQUENCY_MAX;

		m_bIsRecording = true;
	}
	void VulkanCommandBuffer::EndRecording(RenderOrder eOrder, bool bEndPass /*= true*/)
	{
		if(bEndPass)
			m_xCurrentCmdBuffer.endRenderPass();
		RendererAPI* pxRendererAPI = VulkanRenderer::GetInstance()->m_pxRendererAPI;
		m_xCurrentCmdBuffer.end();
		pxRendererAPI->s_axCmdBuffersToSubmit[eOrder].push_back(&m_xCurrentCmdBuffer);

		m_eCurrentBindFreq = RendererAPI::BINDING_FREQUENCY_MAX;

		m_bIsRecording = false;

	}

	void VulkanCommandBuffer::SetVertexBuffer(VertexBuffer* xVertexBuffer, uint32_t uBindPoint /*= 0*/)
	{
		VulkanVertexBuffer* pxVkVertexBuffer = dynamic_cast<VulkanVertexBuffer*>(xVertexBuffer);
		//TODO: offsets
		vk::DeviceSize offsets[] = { 0 };
		m_xCurrentCmdBuffer.bindVertexBuffers(uBindPoint, 1, &dynamic_cast<VulkanBuffer*>(pxVkVertexBuffer->m_pxVertexBuffer)->m_xBuffer, offsets);
	}
	void VulkanCommandBuffer::SetIndexBuffer(IndexBuffer* xIndexBuffer)
	{
		VulkanIndexBuffer* pxVkIndexBuffer = dynamic_cast<VulkanIndexBuffer*>(xIndexBuffer);
		m_xCurrentCmdBuffer.bindIndexBuffer(dynamic_cast<VulkanBuffer*>(pxVkIndexBuffer->m_pxIndexBuffer)->m_xBuffer, 0, vk::IndexType::eUint32);
	}
	void VulkanCommandBuffer::Draw(uint32_t uNumIndices, uint32_t uNumInstances /*= 1*/, uint32_t uVertexOffset /*= 0*/, uint32_t uIndexOffset /*= 0*/, uint32_t uInstanceOffset /*= 0*/)
	{
		if (m_pxRenderer->m_bDisableDrawCalls)
			return;

		if (m_pxCurrentPipeline->m_axDescLayouts.size() > 1) {
			vk::DescriptorSetLayout& xLayout = m_pxCurrentPipeline->m_axDescLayouts[1];
			vk::DescriptorSet xSet = m_pxRenderer->CreateDescriptorSet(xLayout, m_pxRenderer->m_xPerFrameDescriptorPool[m_pxRenderer->m_currentFrame]);

			uint32_t uNumTextures = 0;
			for (uint32_t i = 0; i < MAX_BINDINGS; i++) {
				if (m_xBindings[RendererAPI::BINDING_FREQUENCY_PER_DRAW].m_xTextures[i] != nullptr)
					uNumTextures++;
				else {
#ifdef VCE_DEBUG
					for (uint32_t j = i + 1; j < MAX_BINDINGS; j++)
						VCE_ASSERT(m_xBindings[RendererAPI::BINDING_FREQUENCY_PER_DRAW].m_xTextures[j] == nullptr, "All non null textures must be contiguous");
#endif
					break;
				}

			}

			uint32_t uNumBuffers = 0;
			for (uint32_t i = 0; i < MAX_BINDINGS; i++) {
				if (m_xBindings[RendererAPI::BINDING_FREQUENCY_PER_DRAW].m_xBuffers[i] != nullptr)
					uNumBuffers++;
				else {
#ifdef VCE_DEBUG
					for (uint32_t j = i + 1; j < MAX_BINDINGS; j++)
						VCE_ASSERT(m_xBindings[RendererAPI::BINDING_FREQUENCY_PER_DRAW].m_xBuffers[j] == nullptr, "All non null buffers must be contiguous");
#endif
					break;
				}

			}

			std::vector<vk::DescriptorImageInfo> xTexInfos(uNumTextures);
			std::vector<vk::WriteDescriptorSet> xTexWrites(uNumTextures);
			uint32_t uCount = 0;
			for (uint32_t i = 0; i < uNumTextures; i++) {
				Texture* pxTex = m_xBindings[RendererAPI::BINDING_FREQUENCY_PER_DRAW].m_xTextures[i];
				VulkanTexture2D* pxVkTex = dynamic_cast<VulkanTexture2D*>(pxTex);

				vk::DescriptorImageInfo& xInfo = xTexInfos.at(uCount)
					.setSampler(pxVkTex->m_xSampler)
					.setImageView(pxVkTex->m_xImageView)
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

				vk::WriteDescriptorSet& xWrite = xTexWrites.at(uCount)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setDstSet(xSet)
					.setDstBinding(uCount)
					.setDstArrayElement(0)
					.setDescriptorCount(1)
					.setPImageInfo(&xInfo);

				uCount++;
			}

			m_pxRenderer->GetDevice().updateDescriptorSets(xTexWrites.size(), xTexWrites.data(), 0, nullptr);

			std::vector<vk::DescriptorBufferInfo> xBufferInfos(uNumBuffers);
			std::vector<vk::WriteDescriptorSet> xBufferWrites(uNumBuffers);
			for (uint32_t i = 0; i < uNumBuffers; i++) {
				Buffer* pxBuf = m_xBindings[RendererAPI::BINDING_FREQUENCY_PER_DRAW].m_xBuffers[i];
				VulkanBuffer* pxVkBuf = dynamic_cast<VulkanBuffer*>(pxBuf);

				vk::DescriptorBufferInfo& xInfo = xBufferInfos.at(uCount)
					.setBuffer(pxVkBuf->m_xBuffer)
					.setOffset(0)
					.setRange(pxVkBuf->GetSize());

				vk::WriteDescriptorSet& xWrite = xTexWrites.at(uCount)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setDstSet(xSet)
					.setDstBinding(uCount)
					.setDstArrayElement(0)
					.setDescriptorCount(1)
					.setPBufferInfo(&xInfo);

				uCount++;
			}

			m_pxRenderer->GetDevice().updateDescriptorSets(xBufferWrites.size(), xBufferWrites.data(), 0, nullptr);

			m_xCurrentCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pxCurrentPipeline->m_xPipelineLayout, (int)RendererAPI::BINDING_FREQUENCY_PER_DRAW, 1, &xSet, 0, nullptr);
		}
	

		m_pxRenderer->RecordDrawCall();
		m_xCurrentCmdBuffer.drawIndexed(uNumIndices, uNumInstances, uIndexOffset, uVertexOffset, uInstanceOffset);
	}

	void VulkanCommandBuffer::SubmitTargetSetup(const RendererAPI::TargetSetup& xTargetSetup)
	{
		std::string strLookupName = xTargetSetup.m_strName;

		m_xCurrentRenderPass = m_pxRenderer->m_xTargetSetupPasses.at(strLookupName.c_str())->m_xRenderPass;

		
		m_xCurrentFramebuffer = m_pxRenderer->m_xTargetSetupFramebuffers.at(strLookupName.c_str()).at(m_pxRenderer->m_currentFrame);


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

	void VulkanCommandBuffer::SetPipeline(RendererAPI::Pipeline* pxPipeline)
	{
		VulkanPipeline* pxVkPipeline = dynamic_cast<VulkanPipeline*>(pxPipeline);
		m_xCurrentCmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pxVkPipeline->m_xPipeline);
		std::vector<vk::DescriptorSet> axSets;
		//new pipelines (skinned meshes)
		if (pxVkPipeline->m_axDescLayouts.size()) {
			for (const vk::DescriptorSet set : pxVkPipeline->m_axDescSets[m_pxRenderer->m_currentFrame])
				axSets.push_back(set);
		}
		m_xCurrentCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pxVkPipeline->m_xPipelineLayout, 0, axSets.size(), axSets.data(), 0, nullptr);

		m_pxCurrentPipeline = pxVkPipeline;
	}

	void VulkanCommandBuffer::BindTexture(void* pxTexture, uint32_t uBindPoint, uint32_t uSet) {
		VCE_ASSERT(m_eCurrentBindFreq < RendererAPI::BINDING_FREQUENCY_MAX, "Haven't called BeginBind");

		if (m_eCurrentBindFreq == RendererAPI::BINDING_FREQUENCY_PER_FRAME) {
			VulkanTexture2D* pxTex = reinterpret_cast<VulkanTexture2D*>(pxTexture);

			vk::DescriptorImageInfo xInfo = vk::DescriptorImageInfo()
				.setSampler(pxTex->m_xSampler)
				.setImageView(pxTex->m_xImageView)
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

			vk::WriteDescriptorSet xWrite = vk::WriteDescriptorSet()
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				//#TO index 0 for per frame set
				.setDstSet(m_pxCurrentPipeline->m_axDescSets[m_pxRenderer->m_currentFrame][0])
				.setDstBinding(uBindPoint)
				.setDstArrayElement(0)
				.setDescriptorCount(1)
				.setPImageInfo(&xInfo);

			m_pxRenderer->GetDevice().updateDescriptorSets(1, &xWrite, 0, nullptr);
		}
		else if(m_eCurrentBindFreq == RendererAPI::BINDING_FREQUENCY_PER_DRAW)
			m_xBindings[m_eCurrentBindFreq].m_xTextures[uBindPoint] = reinterpret_cast<Texture*>(pxTexture);
	}

	void VulkanCommandBuffer::BindBuffer(void* pxBuffer, uint32_t uBindPoint, uint32_t uSet) {
		VCE_ASSERT(m_eCurrentBindFreq < RendererAPI::BINDING_FREQUENCY_MAX, "Haven't called BeginBind");

		if (m_eCurrentBindFreq == RendererAPI::BINDING_FREQUENCY_PER_FRAME) {
			VulkanBuffer* pxBuf = reinterpret_cast<VulkanBuffer*>(pxBuffer);

			vk::DescriptorBufferInfo xInfo = vk::DescriptorBufferInfo()
				.setBuffer(pxBuf->m_xBuffer)
				.setOffset(0)
				.setRange(pxBuf->GetSize());

			vk::WriteDescriptorSet xWrite = vk::WriteDescriptorSet()
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				//#TO index 0 for per frame set
				.setDstSet(m_pxCurrentPipeline->m_axDescSets[m_pxRenderer->m_currentFrame][0])
				.setDstBinding(uBindPoint)
				.setDescriptorCount(1)
				.setPBufferInfo(&xInfo);

			m_pxRenderer->GetDevice().updateDescriptorSets(1, &xWrite, 0, nullptr);

		}
		else if (m_eCurrentBindFreq == RendererAPI::BINDING_FREQUENCY_PER_DRAW)
			m_xBindings[m_eCurrentBindFreq].m_xBuffers[uBindPoint] = reinterpret_cast<Buffer*>(pxBuffer);
	}

	void VulkanCommandBuffer::PushConstant(void* pData, size_t uSize)
	{
		m_xCurrentCmdBuffer.pushConstants(m_pxCurrentPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, 0, uSize, pData);
	}

	void VulkanCommandBuffer::UploadUniformData(void* pData, size_t uSize)
	{
		m_pxUniformBuffer->UploadData(pData, uSize, m_pxRenderer->m_currentFrame);
	}

	void VulkanCommandBuffer::BeginBind(RendererAPI::BINDING_FREQUENCY eFreq)
	{
		for (uint32_t i = 0; i < MAX_BINDINGS; i++) {
			m_xBindings[eFreq].m_xBuffers[i] = nullptr;
			m_xBindings[eFreq].m_xTextures[i] = nullptr;
		}
		m_eCurrentBindFreq = eFreq;
	}
}


