#pragma once
#include "VeryCoolEngine/Renderer/Material.h"
#include "VulkanRenderer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace VeryCoolEngine {
	class VulkanMaterial : public Material
	{
	public:
		VulkanMaterial() {};
		VulkanMaterial(const char* szName);
		~VulkanMaterial() override {
			VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
			pxRenderer->GetDevice().destroyDescriptorSetLayout(m_xDescSetLayout);
		};

		void PlatformInit() override;

		vk::DescriptorSet m_xDescSet;
		vk::DescriptorSetLayout m_xDescSetLayout;
	};
}