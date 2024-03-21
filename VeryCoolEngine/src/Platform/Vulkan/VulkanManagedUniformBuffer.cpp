#include "vcepch.h"
#include "VulkanManagedUniformBuffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace VeryCoolEngine {


	//TODO: get rid of last parameter
	VulkanManagedUniformBuffer::VulkanManagedUniformBuffer(uint32_t size, uint8_t numFrames, uint32_t baseBinding) : m_uNumFrames(numFrames), m_uSize(size), m_uBaseBinding(baseBinding)
	{

		VulkanRenderer* pRenderer = VulkanRenderer::GetInstance();

		ppBuffers = new Buffer* [numFrames];
		ppMappedPtrs = new void* [numFrames];
		for (uint8_t i = 0; i < numFrames; i++)
		{
			ppBuffers[i] = new VulkanBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			ppMappedPtrs[i] = VulkanRenderer::GetInstance()->GetDevice().mapMemory(dynamic_cast<VulkanBuffer*>(ppBuffers[i])->m_xDeviceMem, 0, size);
		}


		
	}

	void VulkanManagedUniformBuffer::UploadData(const void* const data, uint32_t size, uint8_t frame, uint32_t offset)
	{
		void* dst = ppMappedPtrs[frame];
		memcpy(dst, data, size);
	}

}


