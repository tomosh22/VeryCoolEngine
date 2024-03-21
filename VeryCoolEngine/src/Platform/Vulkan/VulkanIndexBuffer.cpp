#include "vcepch.h"
#include "VulkanIndexBuffer.h"

namespace VeryCoolEngine {
	
	VulkanIndexBuffer::VulkanIndexBuffer(void* m_puIndices, size_t size) {
		VulkanBuffer pxStagingBuffer = VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		void* pData = xDevice.mapMemory(pxStagingBuffer.m_xDeviceMem, 0, size);
		memcpy(pData, m_puIndices, size);
		xDevice.unmapMemory(pxStagingBuffer.m_xDeviceMem);

		m_pxIndexBuffer = new VulkanBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

		VulkanBuffer::CopyBufferToBuffer(&pxStagingBuffer, dynamic_cast<VulkanBuffer*>(m_pxIndexBuffer), size);
	}
	void VulkanIndexBuffer::UploadData()
	{
	}
	void VulkanIndexBuffer::Bind() const
	{
	}
	void VulkanIndexBuffer::Unbind() const
	{
	}
}