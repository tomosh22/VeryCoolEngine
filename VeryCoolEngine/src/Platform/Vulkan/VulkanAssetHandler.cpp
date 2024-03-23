#include "vcepch.h"
#include "VulkanAssetHandler.h"
#include "VeryCoolEngine/AssetHandling/AssetHandler.h"
#include "VeryCoolEngine/Renderer/Texture.h"
#include "VeryCoolEngine/Renderer/Mesh.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace VeryCoolEngine {
	void VulkanAssetHandler::PlatformInitialiseAssets() {

		VulkanCommandBuffer* pxBlockingCommandBuffer = new VulkanCommandBuffer;
		pxBlockingCommandBuffer->BeginRecording();

		for (auto it = m_xTexture2dMap.begin(); it != m_xTexture2dMap.end(); it++)
			it->second->PlatformInit();
		for (auto it = m_xMaterialMap.begin(); it != m_xMaterialMap.end(); it++)
			it->second->PlatformInit();
		for (auto it = m_xFoliageMaterialMap.begin(); it != m_xFoliageMaterialMap.end(); it++)
			it->second->PlatformInit();
		for (auto it = m_xMeshMap.begin(); it != m_xMeshMap.end(); it++)
			it->second->PlatformInit();
	}
}