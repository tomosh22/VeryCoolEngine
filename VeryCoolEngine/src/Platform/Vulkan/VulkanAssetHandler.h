#pragma once
#include "VeryCoolEngine/AssetHandling/AssetHandler.h"

namespace VeryCoolEngine {
	class VulkanCommandBuffer;
	class VulkanAssetHandler : public AssetHandler {
	public:
		VulkanAssetHandler();
		void PlatformInitialiseAssets() override;

		VulkanCommandBuffer* m_pxBlockingCommandBuffer = nullptr;

	};
}

