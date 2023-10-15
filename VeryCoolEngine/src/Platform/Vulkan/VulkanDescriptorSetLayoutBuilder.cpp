/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "vcepch.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanRenderer.h"

namespace VeryCoolEngine {

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithSamplers(unsigned int count, vk::ShaderStageFlags inShaders) {
		VCE_ASSERT((inShaders & vk::ShaderStageFlagBits::eFragment) == vk::ShaderStageFlagBits::eFragment, "");
		vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
			.setBinding((uint32_t)addedBindings.size())
			.setDescriptorCount(count)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setStageFlags(inShaders);

		addedBindings.emplace_back(binding);

		return *this;
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithUniformBuffers(unsigned int count, vk::ShaderStageFlags inShaders) {
		vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
			.setBinding((uint32_t)addedBindings.size())
			.setDescriptorCount(count)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(inShaders);

		addedBindings.emplace_back(binding);
		return *this;
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithStorageBuffers(unsigned int count, vk::ShaderStageFlags inShaders) {
		vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
			.setBinding((uint32_t)addedBindings.size())
			.setDescriptorCount(count)
			.setDescriptorType(vk::DescriptorType::eStorageBuffer)
			.setStageFlags(inShaders);

		addedBindings.emplace_back(binding);
		return *this;
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithBindlessAccess() {
		usingBindless = true;
		return *this;
	}

	VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::WithDescriptorBufferAccess() {
		usingDescriptorBuffer = true;
		return *this;
	}

	vk::DescriptorSetLayout VulkanDescriptorSetLayoutBuilder::Build(vk::Device device) {
		createInfo.setPBindings(addedBindings.data());
		createInfo.setBindingCount(addedBindings.size());
		vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo;
		std::vector< vk::DescriptorBindingFlags> bindingFlags;

		if (usingBindless) {
			for (int i = 0; i < addedBindings.size(); ++i) {
				bindingFlags.push_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
			}
			bindingFlagsInfo.setBindingFlags(bindingFlags);
			createInfo.pNext = &bindingFlagsInfo;

			createInfo.flags |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
		}
		/*if (usingDescriptorBuffer) {
			createInfo.flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
		}*/

		vk::DescriptorSetLayout layout = device.createDescriptorSetLayout(createInfo);
		return layout;
	}

	static vk::ShaderStageFlags VulkanShaderStage(ShaderStage eStage) {
		switch (eStage) {
		case ShaderStageVertex:
			return vk::ShaderStageFlagBits::eVertex;
		case ShaderStageFragment:
			return vk::ShaderStageFlagBits::eFragment;
		case ShaderStageVertexAndFragment:
			return vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
		}
	}

	vk::DescriptorSetLayout VulkanDescriptorSetLayoutBuilder::FromSpecification(const DescriptorSpecification spec)
	{
		VulkanDescriptorSetLayoutBuilder xBuilder = VulkanDescriptorSetLayoutBuilder();
		if (spec.m_aeSamplerStages.size()) {
			for (auto& [ppxTexture, eStage] : spec.m_aeSamplerStages) {
				xBuilder = xBuilder.WithSamplers(1, VulkanShaderStage(eStage));
			}
		}
		if (spec.m_aeUniformBufferStages.size()) {
			for (auto& [ppxUBO, eStage] : spec.m_aeUniformBufferStages) {
				xBuilder = xBuilder.WithUniformBuffers(1, VulkanShaderStage(eStage));
			}
		}
		return std::move(xBuilder.Build(VulkanRenderer::GetInstance()->GetDevice()));
	}
}