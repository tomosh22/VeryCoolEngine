#include "vcepch.h"
#include "VulkanVertexBuffer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "VeryCoolEngine/Application.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

VeryCoolEngine::VulkanVertexBuffer::VulkanVertexBuffer(void* verts, size_t size)
{
	

	m_pxVertexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	VulkanRenderer* pRenderer = VulkanRenderer::GetInstance();
	vk::Device xDevice = pRenderer->m_device;
	void* pData = xDevice.mapMemory(m_pxVertexBuffer->m_xDeviceMem, 0, size);
	memcpy(pData, verts, size);
	xDevice.unmapMemory(m_pxVertexBuffer->m_xDeviceMem);

}

void VeryCoolEngine::VulkanVertexBuffer::UploadData()
{
}
