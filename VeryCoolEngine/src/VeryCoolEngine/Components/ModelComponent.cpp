#include "vcepch.h"
#include "ModelComponent.h"
#include "VeryCoolEngine/Renderer/Material.h"
#include "VeryCoolEngine/Renderer/Mesh.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	ModelComponent::ModelComponent(std::string strFilename, TransformComponent& xTrans, Entity* xEntity) : m_strFilename(strFilename), m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_pxModel = new VCEModel(strFilename.c_str());
	}
	ModelComponent::ModelComponent(std::string strFilename, const std::string& strMaterialName, TransformComponent& xTrans, Entity* xEntity) : m_strFilename(strFilename), m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_strMaterialName = strMaterialName;
		m_pxModel = new VCEModel();

		//#TO_TODO this can be an emplace_back
		Mesh* mesh = Mesh::FromFile(strFilename.c_str());
		mesh->m_pxMaterial = m_xParentEntity.m_pxParentScene->m_xMaterialMap.at(strMaterialName.c_str());
		m_pxModel->m_apxMeshes.push_back(Mesh::FromFile(strFilename.c_str()));
		m_pxModel->m_apxMeshes.back()->m_pxMaterial = mesh->m_pxMaterial;
	}

	void ModelComponent::Serialize(std::ofstream& xOut) {
		xOut << "ModelComponent\n";
		xOut << m_strMaterialName.c_str() << '\n';
		xOut << m_strFilename.c_str() << '\n';
	}
}