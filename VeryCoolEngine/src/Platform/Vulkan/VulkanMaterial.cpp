#include "vcepch.h"
#include "VulkanMaterial.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanRenderer.h"
#include "VulkanTexture.h"

namespace VeryCoolEngine {

	void VulkanMaterial::PlatformInit()
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		Application* app = Application::GetInstance();
		std::vector<Texture2D*> xTextures;

		if (m_pxAlbedo != nullptr) {
			m_pxAlbedo->PlatformInit();
		}
		else {
			m_pxAlbedo = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxAlbedo);

		if (m_pxBumpMap != nullptr){
			m_pxBumpMap->PlatformInit();
		}
		else {
			m_pxBumpMap = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxBumpMap);

		if (m_pxRoughnessTex != nullptr){
			m_pxRoughnessTex->PlatformInit();
		}
		else {
			m_pxRoughnessTex = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxRoughnessTex);

		if (m_pxMetallicTex != nullptr){
			m_pxMetallicTex->PlatformInit();
		}
		else {
			m_pxMetallicTex = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxMetallicTex);

		if (m_pxHeightmapTex != nullptr){
			m_pxHeightmapTex->PlatformInit();
		}
		else {
			m_pxHeightmapTex = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxHeightmapTex);

		m_uNumTextures = xTextures.size();

		VulkanDescriptorSetLayoutBuilder xDescBuilder = VulkanDescriptorSetLayoutBuilder().WithBindlessAccess();
		for (uint32_t i = 0; i < m_uNumTextures; i++)
			xDescBuilder = xDescBuilder.WithSamplers(1);
		m_xDescSetLayout = xDescBuilder.Build(pxRenderer->GetDevice());

		m_xDescSet = pxRenderer->CreateDescriptorSet(m_xDescSetLayout, pxRenderer->GetDescriptorPool());

		std::vector<vk::DescriptorImageInfo> xInfos(m_uNumTextures);
		std::vector<vk::WriteDescriptorSet> xWrites(m_uNumTextures);
		uint32_t uCount = 0;

		for (Texture2D* pxTex : xTextures) {
			VulkanTexture2D* pxVkTex = dynamic_cast<VulkanTexture2D*>(pxTex);

			vk::DescriptorImageInfo& xInfo = xInfos.at(uCount)
				.setSampler(pxVkTex->m_xSampler)
				.setImageView(pxVkTex->m_xImageView)
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

			vk::WriteDescriptorSet& xWrite = xWrites.at(uCount)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDstSet(m_xDescSet)
				.setDstBinding(uCount)
				.setDstArrayElement(0)
				.setDescriptorCount(1)
				.setPImageInfo(&xInfo);

			uCount++;
		}

		pxRenderer->GetDevice().updateDescriptorSets(xWrites.size(), xWrites.data(), 0, nullptr);

		m_bInitialised = true;

	}

}