#include "vcepch.h"
#include "ModelComponent.h"
#include "VeryCoolEngine/Renderer/Material.h"
#include "VeryCoolEngine/Renderer/Mesh.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	ModelComponent::ModelComponent(std::string strFilename, TransformComponent& xTrans) : m_strFilename(strFilename), m_xTransRef(xTrans) {
		m_pxModel = new VCEModel(strFilename.c_str());
	}
	ModelComponent::ModelComponent(std::string strFilename, Material* pxMaterial, TransformComponent& xTrans) : m_strFilename(strFilename), m_xTransRef(xTrans) {
		m_pxModel = new VCEModel();

		//#TO_TODO this can be an emplace_back
		Mesh* mesh = Mesh::FromFile(strFilename.c_str());
		mesh->m_pxMaterial = pxMaterial;
		m_pxModel->m_apxMeshes.push_back(Mesh::FromFile(strFilename.c_str()));
		m_pxModel->m_apxMeshes.back()->m_pxMaterial = pxMaterial;

		//#TO_TODO: get rid of this once the renderer gets transforms from the ECS
		m_pxModel->m_pxTransform = xTrans.m_pxPhysicsTransform;
	}
}