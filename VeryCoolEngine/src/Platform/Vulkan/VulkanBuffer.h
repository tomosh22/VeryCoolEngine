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
			VulkanRenderer::GetInstance()->m_device.destroyBuffer(m_xBuffer);
			VulkanRenderer::GetInstance()->m_device.freeMemory(m_xDeviceMem);
		}

		VulkanBuffer(vk::DeviceSize uSize, vk::BufferUsageFlagBits eUsageFlags, vk::MemoryPropertyFlags eMemProperties);

		vk::Buffer m_xBuffer;
		vk::DeviceMemory m_xDeviceMem;
	private:
		
	};

}

