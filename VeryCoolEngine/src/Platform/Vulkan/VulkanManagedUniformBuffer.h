#pragma once
#include "VeryCoolEngine/Renderer/ManagedUniformBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace VeryCoolEngine {

	class VulkanManagedUniformBuffer : public ManagedUniformBuffer
	{
	public:
		VulkanManagedUniformBuffer() = default;
		VulkanManagedUniformBuffer(const VulkanManagedUniformBuffer& other) = delete;
		~VulkanManagedUniformBuffer() {
			for (uint8_t i = 0; i < m_uNumFrames; i++)
			{
				delete ppBuffers[i];
			}
			delete[] ppBuffers;
			delete[] ppMappedPtrs;
		}

		VulkanManagedUniformBuffer(uint32_t size, uint8_t numFrames, uint32_t baseBinding);

		void UploadData(const void* const data, uint32_t size, uint8_t frame, uint32_t offset = 0) override;

		VulkanBuffer** ppBuffers;

		void** ppMappedPtrs;

		uint8_t m_uNumFrames;
		uint32_t m_uSize;
		uint32_t m_uBaseBinding;

		
	};

}