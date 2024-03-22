#include "vcepch.h"
#include "VulkanVertexBuffer.h"
#include "VeryCoolEngine/Application.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

VeryCoolEngine::VulkanVertexBuffer::VulkanVertexBuffer(void* pData, size_t size, bool bDeviceLocal /*= true*/)
{
	if (bDeviceLocal) {
		VulkanBuffer pxStagingBuffer = VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		

		m_pxVertexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

		if (pData) {

			vk::Device& xDevice = VulkanRenderer::GetInstance()->GetDevice();
			void* pMappedData = xDevice.mapMemory(pxStagingBuffer.m_xDeviceMem, 0, size);
			memcpy(pMappedData, pData, size);
			xDevice.unmapMemory(pxStagingBuffer.m_xDeviceMem);

			VulkanBuffer::CopyBufferToBuffer(&pxStagingBuffer, dynamic_cast<VulkanBuffer*>(m_pxVertexBuffer), size);
		}
	}
	else {
		m_pxVertexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		if (pData) {
			vk::Device& xDevice = VulkanRenderer::GetInstance()->GetDevice();
			void* pMappedData = xDevice.mapMemory(dynamic_cast<VulkanBuffer*>(m_pxVertexBuffer)->m_xDeviceMem, 0, size);
			memcpy(pMappedData, pData, size);
			xDevice.unmapMemory(dynamic_cast<VulkanBuffer*>(m_pxVertexBuffer)->m_xDeviceMem);
		}
	}

}

void VeryCoolEngine::VulkanVertexBuffer::UploadData()
{
}
