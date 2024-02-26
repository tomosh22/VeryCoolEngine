#pragma once
#include "VeryCoolEngine/PlatformTypes.h"
#include "VeryCoolEngine/Renderer/Texture.h"

namespace VeryCoolEngine {
	class Mesh;
	class Material;
	class AssetHandler
	{
	public:
		AssetHandler() = default;
		~AssetHandler() {}

		void LoadAssetsFromFile(const std::string& strFile);

		void PlatformInitialiseAssets();

		void AddTexture2D(GUID xGUID, const std::string& strPath, TextureStreamPriority eStreamPrio);
		void AddMesh(GUID xGUID, const std::string& strPath);
		void AddMaterial(GUID xGUID, const std::string& strName, GUID xAlbedoGUID, GUID xBumpMapGUID, GUID xRoughnessTexGUID, GUID xMetallicTexGUID, GUID xHeightmapTexGUID);

		Texture2D* GetTexture2D(GUID xGUID);
		Texture2D* TryGetTexture2D(GUID xGUID);
		Mesh* GetMesh(GUID xGUID);
		Mesh* TryGetMesh(GUID xGUID);
		Material* GetMaterial(GUID xGUID);
		Material* TryGetMaterial(GUID xGUID);

		Texture2D* GetTexture2D(const std::string& strName);
		Texture2D* TryGetTexture2D(const std::string& strName);
		Mesh* GetMesh(const std::string& strName);
		Mesh* TryGetMesh(const std::string& strName);
		Material* GetMaterial(const std::string& strName);
		Material* TryGetMaterial(const std::string& strName);

		void DeleteTexture2D(GUID xGUID);
		void DeleteMesh(GUID xGUID);
		void DeleteMaterial(GUID xGUID);

		void DeleteTexture2D(const std::string& strName);
		void DeleteMesh(const std::string& strName);
		void DeleteMaterial(const std::string& strName);

		std::unordered_map<GuidType, Texture2D*> m_xTexture2dMap;
		std::unordered_map<GuidType, Material*> m_xMaterialMap;
		std::unordered_map<GuidType, Mesh*> m_xMeshMap;

		std::unordered_map<std::string, Texture2D*> m_xTexture2dNameMap;
		std::unordered_map<std::string, Material*> m_xMaterialNameMap;
		std::unordered_map<std::string, Mesh*> m_xMeshNameMap;
	};

}