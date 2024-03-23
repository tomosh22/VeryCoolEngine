#include "vcepch.h"
#include "VulkanVertexBuffer.h"
#include "VeryCoolEngine/Application.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

VeryCoolEngine::VulkanVertexBuffer::VulkanVertexBuffer(void* pData, size_t size, bool bDeviceLocal /*= true*/)
{
	VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
	if (bDeviceLocal) {
		VulkanBuffer pxStagingBuffer = VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		

		//m_pxVertexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
		m_pxVertexBuffer = pxRenderer->m_pxMemoryManager->AllocateBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, VulkanMemoryManager::GPU_RESIDENT);

		if (pData) {
			pxRenderer->m_pxMemoryManager->UploadData(m_pxVertexBuffer, pData, size);
		}
	}
	else {
		m_pxVertexBuffer = pxRenderer->m_pxMemoryManager->AllocateBuffer(size, vk::BufferUsageFlagBits::eVertexBuffer, VulkanMemoryManager::CPU_RESIDENT);

		if (pData) {
			pxRenderer->m_pxMemoryManager->UploadData(m_pxVertexBuffer, pData, size);
		}
	}

}

void VeryCoolEngine::VulkanVertexBuffer::UploadData()
{
}
