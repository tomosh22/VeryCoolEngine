#include "vcepch.h"
#include "TerrainComponent.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	TerrainComponent::TerrainComponent(GUID xHeightmapGUID, GUID xMaterialGUID, TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_pxHeightmap = Application::GetInstance()->m_xAssetHandler.GetTexture2D(xHeightmapGUID);
		m_pxMaterial = Application::GetInstance()->m_xAssetHandler.GetMaterial(xMaterialGUID);
	}
}