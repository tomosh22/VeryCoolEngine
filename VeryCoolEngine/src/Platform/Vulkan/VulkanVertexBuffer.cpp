#include "vcepch.h"
#include "VulkanVertexBuffer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "VeryCoolEngine/Application.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

VeryCoolEngine::VulkanVertexBuffer::VulkanVertexBuffer(void* verts, size_t size)
{
	m_pxStagingBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
	void* pData = xDevice.mapMemory(m_pxStagingBuffer->m_xDeviceMem, 0, size);
	memcpy(pData, verts, size);
	xDevice.unmapMemory(m_pxStagingBuffer->m_xDeviceMem);

	m_pxVertexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	VulkanBuffer::CopyBufferToBuffer(m_pxStagingBuffer, m_pxVertexBuffer, size);

}

void VeryCoolEngine::VulkanVertexBuffer::UploadData()
{
}
