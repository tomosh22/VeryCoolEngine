#include "vcepch.h"
#include "VulkanMaterial.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanRenderer.h"
#include "VulkanTexture.h"

namespace VeryCoolEngine {
	VulkanMaterial::VulkanMaterial(const char* szName)
	{
		//TODO: don't make so many std::strings
		SetAlbedo(Texture2D::Create((std::string(szName) + "/diffuse.jpg").c_str(), false));
		SetBumpMap(Texture2D::Create((std::string(szName) + "/normal.jpg").c_str(), false));
		SetRoughness(Texture2D::Create((std::string(szName) + "/roughness.jpg").c_str(), false));
		SetMetallic(Texture2D::Create((std::string(szName) + "/metallic.jpg").c_str(), false));
		SetHeightmap(Texture2D::Create((std::string(szName) + "/height.jpg").c_str(), false));
	}
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


#pragma mark foliage
	VulkanFoliageMaterial::VulkanFoliageMaterial(const char* szName)
	{
		//TODO: don't make so many std::strings
		SetAlbedo(Texture2D::Create((std::string(szName) + "/diffuse.jpg").c_str(), false));
		SetBumpMap(Texture2D::Create((std::string(szName) + "/normal.jpg").c_str(), false));
		SetRoughness(Texture2D::Create((std::string(szName) + "/roughness.jpg").c_str(), false));
		SetHeightmap(Texture2D::Create((std::string(szName) + "/height.jpg").c_str(), false));
		SetAlpha(Texture2D::Create((std::string(szName) + "/alpha.jpg").c_str(), false));
		SetTranslucency(Texture2D::Create((std::string(szName) + "/translucency.jpg").c_str(), false));
	}
	void VulkanFoliageMaterial::PlatformInit()
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

		if (m_pxBumpMap != nullptr) {
			m_pxBumpMap->PlatformInit();
		}
		else {
			m_pxBumpMap = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxBumpMap);

		if (m_pxRoughnessTex != nullptr) {
			m_pxRoughnessTex->PlatformInit();
		}
		else {
			m_pxRoughnessTex = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxRoughnessTex);

		if (m_pxAlphaTex != nullptr) {
			m_pxAlphaTex->PlatformInit();
		}
		else {
			m_pxAlphaTex = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxAlphaTex);

		if (m_pxTranslucencyTex != nullptr) {
			m_pxTranslucencyTex->PlatformInit();
		}
		else {
			m_pxTranslucencyTex = app->m_pxBlankTexture2D;
		}
		xTextures.push_back(m_pxTranslucencyTex);

		if (m_pxHeightmapTex != nullptr) {
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