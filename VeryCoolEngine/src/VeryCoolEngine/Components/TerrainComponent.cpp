#include "vcepch.h"
#include "TerrainComponent.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Renderer/HeightmapTexture.h"

namespace VeryCoolEngine {
	TerrainComponent::TerrainComponent(GUID xHeightmapGUID, GUID xMaterialGUID, int32_t iX, int32_t iY, TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity), m_iCoordX(iX), m_iCoordY(iY) {
		m_pxHeightmap = HeightmapTexture::Create(xHeightmapGUID);
		m_pxMaterial = Application::GetInstance()->m_xAssetHandler.GetMaterial(xMaterialGUID);
	}
}