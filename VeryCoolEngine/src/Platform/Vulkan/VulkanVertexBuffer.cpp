#include "vcepch.h"
#include "VulkanVertexBuffer.h"
#include "VeryCoolEngine/Application.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

VeryCoolEngine::VulkanVertexBuffer::VulkanVertexBuffer(void* m_pVerts, size_t size)
{
	VulkanBuffer pxStagingBuffer = VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
	void* pData = xDevice.mapMemory(pxStagingBuffer.m_xDeviceMem, 0, size);
	memcpy(pData, m_pVerts, size);
	xDevice.unmapMemory(pxStagingBuffer.m_xDeviceMem);

	m_pxVertexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	VulkanBuffer::CopyBufferToBuffer(&pxStagingBuffer, dynamic_cast<VulkanBuffer*>(m_pxVertexBuffer), size);

}

void VeryCoolEngine::VulkanVertexBuffer::UploadData()
{
}
