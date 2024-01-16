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

	Material* Material::Create(const char* szName) {
#ifdef VCE_VULKAN
		return new VulkanMaterial(szName);
#endif
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


}