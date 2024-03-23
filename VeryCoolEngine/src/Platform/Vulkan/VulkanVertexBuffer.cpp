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

		

		m_pxVertexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

		if (pData) {

			vk::Device& xDevice = pxRenderer->GetDevice();
			void* pMappedData = xDevice.mapMemory(pxStagingBuffer.m_xDeviceMem, 0, size);
			memcpy(pMappedData, pData, size);
			xDevice.unmapMemory(pxStagingBuffer.m_xDeviceMem);

			VulkanBuffer::CopyBufferToBuffer(&pxStagingBuffer, dynamic_cast<VulkanBuffer*>(m_pxVertexBuffer), size);
		}
	}
	else {
		m_pxVertexBuffer = pxRenderer->m_pxMemoryManager->AllocateBuffer(size, vk::BufferUsageFlagBits::eVertexBuffer, VulkanMemoryManager::CPU_RESIDENT);

		if (pData) {
			vk::Device& xDevice = pxRenderer->GetDevice();
			void* pMappedData = pxRenderer->m_pxMemoryManager->MapMemory(m_pxVertexBuffer);
			memcpy(pMappedData, pData, size);
			pxRenderer->m_pxMemoryManager->UnmapMemory(m_pxVertexBuffer);
		}
	}

}

void VeryCoolEngine::VulkanVertexBuffer::UploadData()
{
}
