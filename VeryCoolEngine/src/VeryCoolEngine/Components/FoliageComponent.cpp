#include "vcepch.h"
#include "FoliageComponent.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine{
	FoliageComponent::FoliageComponent(GUID xMaterialGUID, glm::vec3 xPos, TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity), m_xPos(xPos) {
		m_pxMaterial = Application::GetInstance()->m_pxAssetHandler->GetFoliageMaterial(xMaterialGUID);
	}
}