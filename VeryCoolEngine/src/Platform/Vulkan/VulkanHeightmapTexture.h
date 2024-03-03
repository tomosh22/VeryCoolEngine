#pragma once
#include "VeryCoolEngine/Renderer/HeightmapTexture.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace VeryCoolEngine {
	class VulkanHeightmapTexture : public HeightmapTexture
	{
	public:
		VulkanHeightmapTexture(GUID xTextureGUID);
		~VulkanHeightmapTexture() {
			VulkanRenderer::GetInstance()->GetDevice().destroySampler(m_xSampler);
		}

		void PlatformInit() override;

		vk::DescriptorSet m_xDescSet;
		vk::DescriptorSetLayout m_xDescSetLayout;
		vk::Sampler m_xSampler;
	};

}