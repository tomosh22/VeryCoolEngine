#include "vcepch.h"
#include "VulkanHeightmapTexture.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "VeryCoolEngine/Application.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanTexture.h"

namespace VeryCoolEngine {
	VulkanHeightmapTexture::VulkanHeightmapTexture(GUID xTextureGUID)
	{
		SetTexture(xTextureGUID);
		Application::GetInstance()->m_apxHeightmapTextures.push_back(this);
	}

	void VulkanHeightmapTexture::PlatformInit()
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		Application* app = Application::GetInstance();

		VulkanDescriptorSetLayoutBuilder xDescBuilder = VulkanDescriptorSetLayoutBuilder().WithBindlessAccess();
		xDescBuilder = xDescBuilder.WithSamplers(1);
		m_xDescSetLayout = xDescBuilder.Build(pxRenderer->GetDevice());

		m_xDescSet = pxRenderer->CreateDescriptorSet(m_xDescSetLayout, pxRenderer->GetDescriptorPool());

		m_pxTexture->PlatformInit();
		VulkanTexture2D* pxVkTex = dynamic_cast<VulkanTexture2D*>(m_pxTexture);

		vk::Device xDevice = pxRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pxRenderer->GetPhysicalDevice();


		//todo all these should be parameters
		vk::SamplerCreateInfo xSamplerInfo = vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eNearest)
			.setMinFilter(vk::Filter::eNearest)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(xPhysDevice.getProperties().limits.maxSamplerAnisotropy)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eNearest)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMaxLod(0);

		m_xSampler = xDevice.createSampler(xSamplerInfo);

		vk::DescriptorImageInfo xInfo = vk::DescriptorImageInfo()
			.setSampler(m_xSampler)
			.setImageView(pxVkTex->m_xImageView)
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::WriteDescriptorSet xWrite = vk::WriteDescriptorSet()
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDstSet(m_xDescSet)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setPImageInfo(&xInfo);

		pxRenderer->GetDevice().updateDescriptorSets(1, &xWrite, 0, nullptr);

		m_bInitialised = true;

	}

}