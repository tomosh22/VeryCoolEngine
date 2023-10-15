#include "vcepch.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderer.h"

namespace VeryCoolEngine {

	VulkanRenderPass::VulkanRenderPass() {

		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
	
		vk::AttachmentDescription colorAttachment = vk::AttachmentDescription()
			.setFormat(VulkanRenderer::GetInstance()->GetSwapchainFormat())
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentDescription depthAttachment = vk::AttachmentDescription()
			.setFormat(vk::Format::eD32Sfloat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::SubpassDescription subpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&colorAttachmentRef)
			.setPDepthStencilAttachment(&depthAttachmentRef);

		vk::SubpassDependency dependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits::eNone)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

		vk::AttachmentDescription axAttachments[2]{ colorAttachment,depthAttachment };

		vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(2)
			.setPAttachments(axAttachments)
			.setSubpassCount(1)
			.setPSubpasses(&subpass)
			.setDependencyCount(1)
			.setPDependencies(&dependency);
		m_xRenderPass = xDevice.createRenderPass(renderPassInfo);
	}
}
