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
		VulkanMaterial(GUID xAlbedoGUID, GUID xBumpMapGUID, GUID xRoughnessTexGUID, GUID xMetallicTexGUID, GUID xHeightmapTexGUID);
		~VulkanMaterial() override {
			VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
			pxRenderer->GetDevice().destroyDescriptorSetLayout(m_xDescSetLayout);
		};

		void PlatformInit() override;

		vk::DescriptorSet m_xDescSet;
		vk::DescriptorSetLayout m_xDescSetLayout;
	};

	class VulkanFoliageMaterial : public FoliageMaterial
	{
	public:
		VulkanFoliageMaterial() {};
		VulkanFoliageMaterial(const char* szName);
		~VulkanFoliageMaterial() override {
			VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
			pxRenderer->GetDevice().destroyDescriptorSetLayout(m_xDescSetLayout);
		};

		void PlatformInit() override;

		vk::DescriptorSet m_xDescSet;
		vk::DescriptorSetLayout m_xDescSetLayout;
	};
}