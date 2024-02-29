#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/PlatformTypes.h"

namespace VeryCoolEngine {
	class Texture2D;
	class Material;
	class TerrainComponent
	{
	public:
		TerrainComponent(GUID xMeshGUID, GUID xMaterialGUID, TransformComponent& xTrans, Entity* xEntity);
		TerrainComponent() = delete;

		TransformComponent& GetTransformRef() { return m_xTransRef; }

		Entity& GetParentEntity() { return m_xParentEntity; }
	private:
		Texture2D* m_pxHeightmap;
		Material* m_pxMaterial;

		TransformComponent& m_xTransRef;
		Entity m_xParentEntity;
	};
}
