#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanTexture.h"
namespace VeryCoolEngine {

	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer() = default;
		VulkanBuffer(const VulkanBuffer& other) = delete;
		~VulkanBuffer() {
			VulkanRenderer::GetInstance()->GetDevice().destroyBuffer(m_xBuffer);
			VulkanRenderer::GetInstance()->GetDevice().freeMemory(m_xDeviceMem);
		}

		VulkanBuffer(vk::DeviceSize uSize, vk::BufferUsageFlags eUsageFlags, vk::MemoryPropertyFlags eMemProperties);

		void UploadData(void* pData, uint32_t uSize) override;

		static void CopyBufferToBuffer(VulkanBuffer* pxSrc, VulkanBuffer* pxDst, size_t uSize);
		static void CopyBufferToImage(VulkanBuffer* pxSrc, VulkanTexture2D* pxDst, bool bAsyncLoader = false);

		vk::Buffer m_xBuffer;
		vk::DeviceMemory m_xDeviceMem;
	private:
		
	};

}

