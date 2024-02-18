#include "vcepch.h"
#include "AssetHandler.h"
#include "VeryCoolEngine/Renderer/Texture.h"
#include "VeryCoolEngine/Renderer/Mesh.h"

namespace VeryCoolEngine {

	void AssetHandler::LoadAssetsFromFile(const std::string& strFile) {
		std::ifstream xIn(strFile);
		std::string strLine;
		while (std::getline(xIn, strLine)) {
			if (strLine == "Texture2D") {
				std::string strGUID;
				std::string strFile;
				std::getline(xIn, strGUID);
				std::getline(xIn, strFile);
				GUID xGUID(strtoull(strGUID.c_str(), nullptr, 10));
				AddTexture2D(xGUID, strFile);
			}
			if (strLine == "Material") {
				std::string strName;
				std::string strGUID;
				std::string strAlbedoGUID;
				std::string strBumpMapGUID;
				std::string strRoughnessTexGUID;
				std::string strMetallicTexGUID;
				std::string strHeightmapTexGUID;
				std::getline(xIn, strName);
				std::getline(xIn, strGUID);
				std::getline(xIn, strAlbedoGUID);
				std::getline(xIn, strBumpMapGUID);
				std::getline(xIn, strRoughnessTexGUID);
				std::getline(xIn, strMetallicTexGUID);
				std::getline(xIn, strHeightmapTexGUID);
				GUID xGUID(strtoull(strGUID.c_str(), nullptr, 10));
				GUID xAlbedoGUID(strtoull(strAlbedoGUID.c_str(), nullptr, 10));
				GUID xBumpMapGUID(strtoull(strBumpMapGUID.c_str(), nullptr, 10));
				GUID xRoughnessTexGUID(strtoull(strRoughnessTexGUID.c_str(), nullptr, 10));
				GUID xMetallicTexGUID(strtoull(strMetallicTexGUID.c_str(), nullptr, 10));
				GUID xHeightmapGUID(strtoull(strHeightmapTexGUID.c_str(), nullptr, 10));
				AddMaterial(xGUID, strName, xAlbedoGUID, xBumpMapGUID, xRoughnessTexGUID, xMetallicTexGUID, xHeightmapGUID);
			}
			if (strLine == "Mesh") {
				std::string strGUID;
				std::string strFile;
				std::getline(xIn, strGUID);
				std::getline(xIn, strFile);
				GUID xGUID(strtoull(strGUID.c_str(), nullptr, 10));
				AddMesh(xGUID, strFile);
			}
		}
	}

	void AssetHandler::PlatformInitialiseAssets() {
		for (auto it = m_xTexture2dMap.begin(); it != m_xTexture2dMap.end(); it++)
			it->second->PlatformInit();
		for (auto it = m_xMaterialMap.begin(); it != m_xMaterialMap.end(); it++)
			it->second->PlatformInit();
		for (auto it = m_xMeshMap.begin(); it != m_xMeshMap.end(); it++)
			it->second->PlatformInit();
	}

	void AssetHandler::AddTexture2D(GUID xGUID, const std::string& strPath) {
		VCE_ASSERT(m_xTexture2dMap.find(xGUID.m_uGuid) == m_xTexture2dMap.end(), "Texture2D guid already exists");
		m_xTexture2dMap.insert({xGUID.m_uGuid, Texture2D::Create(strPath)});
		VCE_ASSERT(m_xTexture2dNameMap.find(strPath) == m_xTexture2dNameMap.end(), "Mesh name already exists");
		m_xTexture2dNameMap.insert({ strPath, m_xTexture2dMap.at(xGUID.m_uGuid) });
	}
	void AssetHandler::AddMesh(GUID xGUID, const std::string& strPath) {
		VCE_ASSERT(m_xMeshMap.find(xGUID.m_uGuid) == m_xMeshMap.end(), "Mesh guid already exists");
		m_xMeshMap.insert({ xGUID.m_uGuid, Mesh::FromFile(strPath) });
		VCE_ASSERT(m_xMeshNameMap.find(strPath) == m_xMeshNameMap.end(), "Mesh name already exists");
		m_xMeshNameMap.insert({ strPath, m_xMeshMap.at(xGUID.m_uGuid) });
	}
	void AssetHandler::AddMaterial(GUID xGUID, const std::string& strName, GUID xAlbedoGUID, GUID xBumpMapGUID, GUID xRoughnessTexGUID, GUID xMetallicTexGUID, GUID xHeightmapTexGUID) {
		VCE_ASSERT(m_xMaterialMap.find(xGUID.m_uGuid) == m_xMaterialMap.end(), "Material guid already exists");
		m_xMaterialMap.insert({ xGUID.m_uGuid, Material::Create(xAlbedoGUID, xBumpMapGUID, xRoughnessTexGUID, xMetallicTexGUID, xHeightmapTexGUID)});
		VCE_ASSERT(m_xMaterialNameMap.find(strName) == m_xMaterialNameMap.end(), "Material name already exists");
		m_xMaterialNameMap.insert({ strName, m_xMaterialMap.at(xGUID.m_uGuid)});
	}

	Texture2D* AssetHandler::GetTexture2D(GUID xGUID) {
		VCE_ASSERT(m_xTexture2dMap.find(xGUID.m_uGuid) != m_xTexture2dMap.end(), "Texture2D doesn't exist");
		return m_xTexture2dMap.at(xGUID.m_uGuid);
	}
	Texture2D* AssetHandler::TryGetTexture2D(GUID xGUID) {
		if (m_xTexture2dMap.find(xGUID.m_uGuid) != m_xTexture2dMap.end())
			return m_xTexture2dMap.at(xGUID.m_uGuid);
		else
			return nullptr;
	}
	Mesh* AssetHandler::GetMesh(GUID xGUID) {
		VCE_ASSERT(m_xMeshMap.find(xGUID.m_uGuid) != m_xMeshMap.end(), "Mesh doesn't exist");
		return m_xMeshMap.at(xGUID.m_uGuid);
	}
	Mesh* AssetHandler::TryGetMesh(GUID xGUID) {
		if (m_xMeshMap.find(xGUID.m_uGuid) != m_xMeshMap.end())
			return m_xMeshMap.at(xGUID.m_uGuid);
		else
			return nullptr;
	}
	Material* AssetHandler::GetMaterial(GUID xGUID) {
		VCE_ASSERT(m_xMaterialMap.find(xGUID.m_uGuid) != m_xMaterialMap.end(), "Material doesn't exist");
		return m_xMaterialMap.at(xGUID.m_uGuid);
	}
	Material* AssetHandler::TryGetMaterial(GUID xGUID) {
		if (m_xMaterialMap.find(xGUID.m_uGuid) != m_xMaterialMap.end())
			return m_xMaterialMap.at(xGUID.m_uGuid);
		else
			return nullptr;
	}

	Texture2D* AssetHandler::GetTexture2D(const std::string& strName) {
		VCE_ASSERT(m_xTexture2dNameMap.find(strName) != m_xTexture2dNameMap.end(), "Texture2D doesn't exist");
		return m_xTexture2dNameMap.at(strName);
	}
	Texture2D* AssetHandler::TryGetTexture2D(const std::string& strName) {
		if (m_xTexture2dNameMap.find(strName) != m_xTexture2dNameMap.end())
			return m_xTexture2dNameMap.at(strName);
		else
			return nullptr;
	}
	Mesh* AssetHandler::GetMesh(const std::string& strName) {
		VCE_ASSERT(m_xMeshNameMap.find(strName) != m_xMeshNameMap.end(), "Mesh doesn't exist");
		return m_xMeshNameMap.at(strName);
	}
	Mesh* AssetHandler::TryGetMesh(const std::string& strName) {
		if (m_xMeshNameMap.find(strName) != m_xMeshNameMap.end())
			return m_xMeshNameMap.at(strName);
		else
			return nullptr;
	}
	Material* AssetHandler::GetMaterial(const std::string& strName) {
		VCE_ASSERT(m_xMaterialNameMap.find(strName) != m_xMaterialNameMap.end(), "Material doesn't exist");
		return m_xMaterialNameMap.at(strName);
	}
	Material* AssetHandler::TryGetMaterial(const std::string& strName) {
		if (m_xMaterialNameMap.find(strName) != m_xMaterialNameMap.end())
			return m_xMaterialNameMap.at(strName);
		else
			return nullptr;
	}

	void AssetHandler::DeleteTexture2D(GUID xGUID) {
		VCE_ASSERT(m_xTexture2dMap.find(xGUID.m_uGuid) != m_xTexture2dMap.end(), "Texture2D doesn't exist");
		VCE_DELETE(m_xTexture2dMap.at(xGUID.m_uGuid));
		m_xTexture2dMap.erase(xGUID.m_uGuid);
	}
	void AssetHandler::DeleteMesh(GUID xGUID) {
		VCE_ASSERT(m_xMeshMap.find(xGUID.m_uGuid) != m_xMeshMap.end(), "Mesh doesn't exist");
		VCE_DELETE(m_xMeshMap.at(xGUID.m_uGuid));
		m_xMeshMap.erase(xGUID.m_uGuid);
	}
	void AssetHandler::DeleteMaterial(GUID xGUID) {
		VCE_ASSERT(m_xMaterialMap.find(xGUID.m_uGuid) != m_xMaterialMap.end(), "Material doesn't exist");
		VCE_DELETE(m_xMaterialMap.at(xGUID.m_uGuid));
		m_xMaterialMap.erase(xGUID.m_uGuid);
	}

	void AssetHandler::DeleteTexture2D(const std::string& strName) {
		VCE_ASSERT(m_xTexture2dNameMap.find(strName) != m_xTexture2dNameMap.end(), "Texture2D doesn't exist");
		VCE_DELETE(m_xTexture2dNameMap.at(strName));
		m_xTexture2dNameMap.erase(strName);
	}
	void AssetHandler::DeleteMesh(const std::string& strName) {
		VCE_ASSERT(m_xMeshNameMap.find(strName) != m_xMeshNameMap.end(), "Mesh doesn't exist");
		VCE_DELETE(m_xMeshNameMap.at(strName));
		m_xMeshNameMap.erase(strName);
	}
	void AssetHandler::DeleteMaterial(const std::string& strName) {
		VCE_ASSERT(m_xMaterialNameMap.find(strName) != m_xMaterialNameMap.end(), "Material doesn't exist");
		VCE_DELETE(m_xMaterialNameMap.at(strName));
		m_xMaterialNameMap.erase(strName);
	}
}