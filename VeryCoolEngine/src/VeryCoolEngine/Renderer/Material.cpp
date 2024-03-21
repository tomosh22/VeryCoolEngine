#include "vcepch.h"
#include "Material.h"
#ifdef VCE_VULKAN
#include "Platform/Vulkan/VulkanMaterial.h"
#endif

namespace VeryCoolEngine {
	Material* Material::Create() {
#ifdef VCE_VULKAN
		return new VulkanMaterial();
#endif
	}

	Material* Material::Create(GUID xAlbedoGUID, GUID xBumpMapGUID, GUID xRoughnessTexGUID, GUID xMetallicTexGUID, GUID xHeightmapTexGUID) {
#ifdef VCE_VULKAN
		return new VulkanMaterial(xAlbedoGUID, xBumpMapGUID, xRoughnessTexGUID, xMetallicTexGUID, xHeightmapTexGUID);
#endif
	}

	void Material::SetAlbedo(GUID xGUID)
	{
		m_pxAlbedo = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void Material::SetBumpMap(GUID xGUID)
	{
		m_pxBumpMap = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void Material::SetRoughness(GUID xGUID)
	{
		m_pxRoughnessTex = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void Material::SetMetallic(GUID xGUID)
	{
		m_pxMetallicTex = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void Material::SetHeightmap(GUID xGUID)
	{
		
		m_pxHeightmapTex = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void Material::SetAlbedo(Texture2D* pxTex)
	{
		m_pxAlbedo = pxTex;
	}

	void Material::SetBumpMap(Texture2D* pxTex)
	{
		m_pxBumpMap = pxTex;
	}

	void Material::SetRoughness(Texture2D* pxTex)
	{
		m_pxRoughnessTex = pxTex;
	}

	void Material::SetMetallic(Texture2D* pxTex)
	{
		m_pxMetallicTex = pxTex;
	}

	void Material::SetHeightmap(Texture2D* pxTex)
	{
		m_pxHeightmapTex = pxTex;
	}

	FoliageMaterial* FoliageMaterial::Create() {
#ifdef VCE_VULKAN
		return new VulkanFoliageMaterial();
#endif
	}

	FoliageMaterial* FoliageMaterial::Create(GUID xAlbedoGUID, GUID xBumpMapGUID, GUID xRoughnessTexGUID, GUID xHeightmapTexGUID, GUID xAlphaTexGUID, GUID xTranslucencyTexGUID) {
#ifdef VCE_VULKAN
		return new VulkanFoliageMaterial(xAlbedoGUID, xBumpMapGUID, xRoughnessTexGUID, xHeightmapTexGUID, xAlphaTexGUID, xTranslucencyTexGUID);
#endif
	}

	void FoliageMaterial::SetAlbedo(GUID xGUID)
	{
		m_pxAlbedo = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void FoliageMaterial::SetBumpMap(GUID xGUID)
	{
		m_pxBumpMap = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void FoliageMaterial::SetHeightmap(GUID xGUID)
	{
		m_pxHeightmapTex = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void FoliageMaterial::SetRoughness(GUID xGUID)
	{
		m_pxRoughnessTex = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void FoliageMaterial::SetAlpha(GUID xGUID)
	{
		m_pxAlphaTex = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}

	void FoliageMaterial::SetTranslucency(GUID xGUID)
	{
		m_pxTranslucencyTex = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}
}