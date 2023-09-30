#include "vcepch.h"
#include "VulkanManagedUniformBuffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace VeryCoolEngine {


	VulkanManagedUniformBuffer::VulkanManagedUniformBuffer(uint32_t size, uint8_t numFrames, uint32_t baseBinding) : m_uNumFrames(numFrames), m_uSize(size), m_uBaseBinding(baseBinding)
	{

		VulkanRenderer* pRenderer = VulkanRenderer::GetInstance();

		ppBuffers = new VulkanBuffer* [numFrames];
		ppMappedPtrs = new void* [numFrames];
		for (uint8_t i = 0; i < numFrames; i++)
		{
			ppBuffers[i] = new VulkanBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			ppMappedPtrs[i] = VulkanRenderer::GetInstance()->GetDevice().mapMemory(ppBuffers[i]->m_xDeviceMem, 0, size);
		}


		vk::DescriptorSetLayoutBinding xBinding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutCreateInfo xCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(1)
			.setPBindings(&xBinding);
		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		m_xDescriptorLayout = xDevice.createDescriptorSetLayout(xCreateInfo);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_axDescriptorSets[i] = pRenderer->CreateDescriptorSet(m_xDescriptorLayout, pRenderer->GetDescriptorPool());
			vk::Buffer& xUniformBuffer = ppBuffers[i]->m_xBuffer;
			uint32_t uSize = m_uSize;
			uint32_t uBinding = m_uBaseBinding;
			vk::DescriptorBufferInfo xBufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(xUniformBuffer)
				.setOffset(0)
				.setRange(uSize);

			vk::WriteDescriptorSet xDescWrite = vk::WriteDescriptorSet()
				.setDstSet(m_axDescriptorSets[i])
				.setDstBinding(uBinding)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setPBufferInfo(&xBufferInfo);

			VulkanRenderer::GetInstance()->GetDevice().updateDescriptorSets(1, &xDescWrite, 0, nullptr);
		}
	}

	void VulkanManagedUniformBuffer::UploadData(const void* const data, uint32_t size, uint8_t frame, uint32_t offset)
	{
		void* dst = ppMappedPtrs[frame];
		memcpy(dst, data, size);
	}

}


