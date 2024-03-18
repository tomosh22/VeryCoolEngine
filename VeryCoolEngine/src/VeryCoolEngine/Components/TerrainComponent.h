#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/PlatformTypes.h"

//width/height that heightmap is divided into
#define TERRAIN_SIZE 64

//multiplier for vertex positions
#define TERRAIN_SCALE 10

namespace VeryCoolEngine {
	class Texture2D;
	class Mesh;
	class Material;
	class HeightmapTexture;
	class TerrainComponent
	{
	public:
		TerrainComponent(GUID xMeshGUID, GUID xMaterialGUID, int32_t iX, int32_t iY, TransformComponent& xTrans, Entity* xEntity);
		TerrainComponent() = delete;

		TransformComponent& GetTransformRef() { return m_xTransRef; }

		Entity& GetParentEntity() { return m_xParentEntity; }

		Mesh* m_pxMesh;
		Material* m_pxMaterial;
		int32_t m_iCoordX;
		int32_t m_iCoordY;
	private:
		

		TransformComponent& m_xTransRef;
		Entity m_xParentEntity;
	};
}
