#include "vcepch.h"
#include "VulkanBuffer.h"

namespace VeryCoolEngine {
	VulkanBuffer::VulkanBuffer(vk::DeviceSize uSize, vk::BufferUsageFlags eUsageFlags, vk::MemoryPropertyFlags eMemProperties)
	{
		VulkanRenderer* pRenderer = VulkanRenderer::GetInstance();
		vk::BufferCreateInfo xInfo = vk::BufferCreateInfo()
			.setSize(uSize)
			.setUsage(eUsageFlags)
			.setSharingMode(vk::SharingMode::eExclusive);

		vk::Device xDevice = pRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pRenderer->GetPhysicalDevice();

		m_xBuffer = xDevice.createBuffer(xInfo);

		vk::MemoryRequirements xRequirements = xDevice.getBufferMemoryRequirements(m_xBuffer);

		uint32_t memoryType = -1;
		for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
			if ((xRequirements.memoryTypeBits & (1 << i)) && (xPhysDevice.getMemoryProperties().memoryTypes[i].propertyFlags & eMemProperties) == eMemProperties) {
				memoryType = i;
				break;
			}
		}
		VCE_ASSERT(memoryType != -1, "couldn't find physical memory type");

		vk::MemoryAllocateInfo xAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(xRequirements.size)
			.setMemoryTypeIndex(memoryType);

		m_xDeviceMem = xDevice.allocateMemory(xAllocInfo);
		xDevice.bindBufferMemory(m_xBuffer, m_xDeviceMem, 0);
	}

	void VulkanBuffer::CopyBufferToBuffer(VulkanBuffer* pxSrc, VulkanBuffer* pxDst, size_t uSize)
	{
		vk::CommandBufferAllocateInfo xCmdInfo = vk::CommandBufferAllocateInfo()
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(VulkanRenderer::GetInstance()->GetCommandPool())
			.setCommandBufferCount(1);
		vk::CommandBuffer xCmd = VulkanRenderer::GetInstance()->GetDevice().allocateCommandBuffers(xCmdInfo)[0];

		vk::CommandBufferBeginInfo xBeginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		xCmd.begin(xBeginInfo);

		vk::BufferCopy xCopyRegion(0,0,uSize);
		xCmd.copyBuffer(pxSrc->m_xBuffer, pxDst->m_xBuffer, xCopyRegion);
		xCmd.end();

		vk::SubmitInfo xSubmitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers(&xCmd);
		VulkanRenderer::GetInstance()->GetGraphicsQueue().submit(1, &xSubmitInfo, VK_NULL_HANDLE);
		VulkanRenderer::GetInstance()->GetGraphicsQueue().waitIdle();
		VulkanRenderer::GetInstance()->GetDevice().freeCommandBuffers(VulkanRenderer::GetInstance()->GetCommandPool(), 1, &xCmd);

	}
}