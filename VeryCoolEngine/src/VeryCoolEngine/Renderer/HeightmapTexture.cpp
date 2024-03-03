#include "vcepch.h"
#include "HeightmapTexture.h"
#include "VeryCoolEngine/Application.h"
#ifdef VCE_VULKAN
#include "Platform/Vulkan/VulkanHeightmapTexture.h"
#endif

namespace VeryCoolEngine {
	HeightmapTexture* HeightmapTexture::Create(GUID xGUID) {
#ifdef VCE_VULKAN
		return new VulkanHeightmapTexture(xGUID);
#endif
	}
	void HeightmapTexture::SetTexture(GUID xGUID) {
		m_pxTexture = Application::GetInstance()->m_xAssetHandler.TryGetTexture2D(xGUID);
	}
}