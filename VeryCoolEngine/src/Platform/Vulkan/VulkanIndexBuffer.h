#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace VeryCoolEngine {

	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer() = default;
		VulkanIndexBuffer(const VulkanIndexBuffer& other) = delete;

		VulkanIndexBuffer(void* indices, size_t size);
		
		VulkanBuffer* m_pxIndexBuffer;
	};

}


