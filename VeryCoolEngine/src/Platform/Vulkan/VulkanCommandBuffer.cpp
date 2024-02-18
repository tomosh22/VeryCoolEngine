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

		m_uCurrentDescSetIndex = 0;
	}
	void VulkanCommandBuffer::EndRecording(bool bSubmit /*= true*/)
	{
		m_xCurrentCmdBuffer.endRenderPass();
		RendererAPI* pxRendererAPI = VulkanRenderer::GetInstance()->m_pxRendererAPI;
		m_xCurrentCmdBuffer.end();
		if (bSubmit) {
			pxRendererAPI->s_xCmdBuffersToSubmit.push_back(&m_xCurrentCmdBuffer);
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
		if (m_pxRenderer->m_bDisableDrawCalls)
			return;
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

	void VulkanCommandBuffer::SetPipeline(void* pxPipeline)
	{
		VulkanPipeline* pxVkPipeline = reinterpret_cast<VulkanPipeline*>(pxPipeline);
		m_xCurrentCmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pxVkPipeline->m_xPipeline);
		std::vector<vk::DescriptorSet> axSets;
		//new pipelines (skinned meshes)
		if (pxVkPipeline->m_axDescLayouts.size()) {
			for (const vk::DescriptorSet set : pxVkPipeline->m_axDescSets[m_pxRenderer->m_currentFrame])
				axSets.push_back(set);
		}
		m_xCurrentCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pxVkPipeline->m_xPipelineLayout, 0, axSets.size(), axSets.data(), 0, nullptr);
		

		m_uCurrentDescSetIndex = axSets.size();

		m_pxCurrentPipeline = pxVkPipeline;
	}

	void VulkanCommandBuffer::BindTexture(void* pxTexture, uint32_t uBindPoint, uint32_t uSet) {
		VulkanTexture2D* pxTex = reinterpret_cast<VulkanTexture2D*>(pxTexture);

		vk::DescriptorImageInfo xInfo = vk::DescriptorImageInfo()
			.setSampler(pxTex->m_xSampler)
			.setImageView(pxTex->m_xImageView)
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::WriteDescriptorSet xWrite = vk::WriteDescriptorSet()
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDstSet(m_pxCurrentPipeline->m_axDescSets[m_pxRenderer->m_currentFrame][uSet])
			.setDstBinding(uBindPoint)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setPImageInfo(&xInfo);

		m_pxRenderer->GetDevice().updateDescriptorSets(1, &xWrite, 0, nullptr);
	}

	void VulkanCommandBuffer::BindBuffer(void* pxBuffer, uint32_t uBindPoint, uint32_t uSet) {

		VulkanBuffer* pxBuf = reinterpret_cast<VulkanBuffer*>(pxBuffer);

		vk::DescriptorBufferInfo xInfo = vk::DescriptorBufferInfo()
			.setBuffer(pxBuf->m_xBuffer)
			.setOffset(0)
			.setRange(pxBuf->GetSize());

		vk::WriteDescriptorSet xWrite = vk::WriteDescriptorSet()
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDstSet(m_pxCurrentPipeline->m_axDescSets[m_pxRenderer->m_currentFrame][uSet])
			.setDstBinding(uBindPoint)
			.setDescriptorCount(1)
			.setPBufferInfo(&xInfo);

		m_pxRenderer->GetDevice().updateDescriptorSets(1, &xWrite, 0, nullptr);

	}

	void VulkanCommandBuffer::PushConstant(void* pData, size_t uSize)
	{
		m_xCurrentCmdBuffer.pushConstants(m_pxCurrentPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, 0, uSize, pData);
	}

	void VulkanCommandBuffer::UploadUniformData(void* pData, size_t uSize)
	{
		m_pxUniformBuffer->UploadData(pData, uSize, m_pxRenderer->m_currentFrame);
	}


	void VulkanCommandBuffer::BindMaterial(Material* pxMaterial, uint32_t uSet)
	{
		VCE_ASSERT(pxMaterial->m_bInitialised, "Binding uninitialised material");
		VulkanMaterial* pxVkMaterial = dynamic_cast<VulkanMaterial*>(pxMaterial);

		m_xCurrentCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pxCurrentPipeline->m_xPipelineLayout, uSet, 1, &pxVkMaterial->m_xDescSet, 0, nullptr);
	}

	void VulkanCommandBuffer::BindAnimation(Mesh* pxMesh, uint32_t uSet)
	{
		VulkanMesh* pxVkMesh = dynamic_cast<VulkanMesh*>(pxMesh);

		m_xCurrentCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pxCurrentPipeline->m_xPipelineLayout, uSet, 1, &pxVkMesh->m_axBoneDescSet[m_pxRenderer->m_currentFrame], 0, nullptr);
	}
}


