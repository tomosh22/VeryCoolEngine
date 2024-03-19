#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/PlatformTypes.h"

namespace VeryCoolEngine {
	class FoliageComponent
	{
		FoliageComponent(GUID xMaterialGUID, glm::vec3 xPos, TransformComponent& xTrans, Entity* xEntity);
	};

}