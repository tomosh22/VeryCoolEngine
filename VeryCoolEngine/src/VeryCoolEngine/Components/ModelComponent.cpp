#include "vcepch.h"
#include "ModelComponent.h"
#include "VeryCoolEngine/Renderer/Material.h"
#include "VeryCoolEngine/Renderer/Mesh.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	ModelComponent::ModelComponent(std::string strFilename, TransformComponent& xTrans, Entity* xEntity) : m_strFilename(strFilename), m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_pxModel = new VCEModel(strFilename.c_str());
	}

	ModelComponent::ModelComponent(GUID xMeshGUID, GUID xMaterialGUID, TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_pxModel = new VCEModel();

		Mesh* mesh = Application::GetInstance()->m_pxAssetHandler->GetMesh(xMeshGUID);
		mesh->m_pxMaterial = Application::GetInstance()->m_pxAssetHandler->GetMaterial(xMaterialGUID);
		m_pxModel->m_apxMeshes.push_back(mesh);
		m_pxModel->m_apxMeshes.back()->m_pxMaterial = mesh->m_pxMaterial;
	}

	ModelComponent::ModelComponent(const std::string& xMeshName, const std::string& xMaterialName, TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_pxModel = new VCEModel();

		Mesh* mesh = Application::GetInstance()->m_pxAssetHandler->GetMesh(xMeshName);
		mesh->m_pxMaterial = Application::GetInstance()->m_pxAssetHandler->GetMaterial(xMaterialName);
		m_pxModel->m_apxMeshes.push_back(mesh);
		m_pxModel->m_apxMeshes.back()->m_pxMaterial = mesh->m_pxMaterial;
	}

	void ModelComponent::Serialize(std::ofstream& xOut) {
		xOut << "ModelComponent\n";
		xOut << m_strMaterialName.c_str() << '\n';
		xOut << m_strFilename.c_str() << '\n';
	}
}