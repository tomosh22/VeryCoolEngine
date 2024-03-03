#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/PlatformTypes.h"

#define TERRAIN_SIZE 128

namespace VeryCoolEngine {
	class Texture2D;
	class Material;
	class HeightmapTexture;
	class TerrainComponent
	{
	public:
		TerrainComponent(GUID xHeightmapGUID, GUID xMaterialGUID, int32_t iX, int32_t iY, TransformComponent& xTrans, Entity* xEntity);
		TerrainComponent() = delete;

		TransformComponent& GetTransformRef() { return m_xTransRef; }

		Entity& GetParentEntity() { return m_xParentEntity; }

		HeightmapTexture* m_pxHeightmap;
		Material* m_pxMaterial;
		int32_t m_iCoordX;
		int32_t m_iCoordY;
	private:
		

		TransformComponent& m_xTransRef;
		Entity m_xParentEntity;
	};
}
