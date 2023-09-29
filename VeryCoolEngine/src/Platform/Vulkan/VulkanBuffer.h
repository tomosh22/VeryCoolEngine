#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
namespace VeryCoolEngine {

	class VulkanBuffer
	{
	public:
		VulkanBuffer() = default;
		VulkanBuffer(const VulkanBuffer& other) = delete;
		~VulkanBuffer() {
			VulkanRenderer::GetInstance()->GetDevice().destroyBuffer(m_xBuffer);
			VulkanRenderer::GetInstance()->GetDevice().freeMemory(m_xDeviceMem);
		}

		VulkanBuffer(vk::DeviceSize uSize, vk::BufferUsageFlags eUsageFlags, vk::MemoryPropertyFlags eMemProperties);

		static void CopyBufferToBuffer(VulkanBuffer* pxSrc, VulkanBuffer* pxDst, size_t uSize);

		vk::Buffer m_xBuffer;
		vk::DeviceMemory m_xDeviceMem;
	private:
		
	};

}

