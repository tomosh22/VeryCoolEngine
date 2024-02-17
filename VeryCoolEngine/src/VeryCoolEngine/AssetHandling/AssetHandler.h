#pragma once
#include "VeryCoolEngine/PlatformTypes.h"

namespace VeryCoolEngine {
	class Texture2D;
	class Mesh;
	class Material;
	class AssetHandler
	{
	public:
		AssetHandler() = default;
		~AssetHandler() {}

		void LoadAssetsFromFile(const std::string& strFile);

		void PlatformInitialiseAssets();

		void AddTexture2D(GUID xGUID, const std::string& strPath);
		void AddMesh(GUID xGUID, const std::string& strPath);
		void AddMaterial(GUID xGUID, GUID xAlbedoGUID, GUID xBumpMapGUID, GUID xRoughnessTexGUID, GUID xMetallicTexGUID, GUID xHeightmapTexGUID);

		Texture2D* GetTexture2D(GUID xGUID);
		Texture2D* TryGetTexture2D(GUID xGUID);
		Mesh* GetMesh(GUID xGUID);
		Mesh* TryGetMesh(GUID xGUID);
		Material* GetMaterial(GUID xGUID);
		Material* TryGetMaterial(GUID xGUID);

		void DeleteTexture2D(GUID xGUID);
		void DeleteMesh(GUID xGUID);
		void DeleteMaterial(GUID xGUID);

		std::unordered_map<GuidType, Texture2D*> m_xTexture2dMap;
		std::unordered_map<GuidType, Material*> m_xMaterialMap;
		std::unordered_map<GuidType, Mesh*> m_xMeshMap;
	};

}