#include "vcepch.h"
#include "TerrainComponent.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	TerrainComponent::TerrainComponent(GUID xMeshGUID, GUID xMaterialGUID, int32_t iX, int32_t iY, TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity), m_iCoordX(iX), m_iCoordY(iY) {
		m_pxMesh = Application::GetInstance()->m_xAssetHandler.GetMesh(xMeshGUID);
		m_pxMaterial = Application::GetInstance()->m_xAssetHandler.GetMaterial(xMaterialGUID);
	}
}