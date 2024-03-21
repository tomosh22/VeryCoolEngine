#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/PlatformTypes.h"

namespace VeryCoolEngine {
	class FoliageMaterial;
	class FoliageComponent
	{
	public:
		FoliageComponent(GUID xMaterialGUID, glm::vec3 xPos, TransformComponent& xTrans, Entity* xEntity);

		FoliageMaterial* m_pxMaterial;
		glm::vec3 m_xPos;
	private:

		TransformComponent& m_xTransRef;
		Entity m_xParentEntity;
	};

}