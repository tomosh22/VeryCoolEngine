#include "vcepch.h"
#include "VulkanBuffer.h"

namespace VeryCoolEngine {
	VulkanBuffer::VulkanBuffer(vk::DeviceSize uSize, vk::BufferUsageFlagBits eUsageFlags, vk::MemoryPropertyFlags eMemProperties)
	{
		VulkanRenderer* pRenderer = VulkanRenderer::GetInstance();
		vk::BufferCreateInfo xInfo = vk::BufferCreateInfo()
			.setSize(uSize)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive);

		vk::Device xDevice = pRenderer->m_device;
		vk::PhysicalDevice xPhysDevice = pRenderer->m_physicalDevice;

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
}