#include "vcepch.h"
#include "VulkanVertexBuffer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "VeryCoolEngine/Application.h"
#include "Platform/Vulkan/VulkanRenderer.h"

VeryCoolEngine::VulkanVertexBuffer::VulkanVertexBuffer(void* verts, size_t size)
{
	VulkanRenderer* pRenderer = VulkanRenderer::GetInstance();
	vk::BufferCreateInfo xInfo = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
		.setSharingMode(vk::SharingMode::eExclusive);

	vk::Device xDevice = pRenderer->m_device;
	vk::PhysicalDevice xPhysDevice = pRenderer->m_physicalDevice;

	m_xVertexBuffer = xDevice.createBuffer(xInfo);

	vk::MemoryRequirements xRequirements = xDevice.getBufferMemoryRequirements(m_xVertexBuffer);

	uint32_t memoryType = -1;
	for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
		if ((xRequirements.memoryTypeBits & (1 << i)) && (xPhysDevice.getMemoryProperties().memoryTypes[i].propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) == (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) {
			memoryType = i;
			break;
		}
	}
	VCE_ASSERT(memoryType != -1, "couldn't find physical memory type");

	vk::MemoryAllocateInfo xAllocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(xRequirements.size)
		.setMemoryTypeIndex(memoryType);

	m_xDeviceMem = xDevice.allocateMemory(xAllocInfo);
	xDevice.bindBufferMemory(m_xVertexBuffer, m_xDeviceMem,0);

	void* pData = xDevice.mapMemory(m_xDeviceMem, 0, size);
	memcpy(pData, verts, size);
	xDevice.unmapMemory(m_xDeviceMem);

}

void VeryCoolEngine::VulkanVertexBuffer::UploadData()
{
}
