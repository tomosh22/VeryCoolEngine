#pragma once
#include "VeryCoolEngine/Renderer/Model.h"
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"

namespace VeryCoolEngine {
	class Material;
	class ModelComponent
	{
	public:
		ModelComponent(std::string strFilename, TransformComponent& xTrans, Entity* xEntity);
		ModelComponent(std::string strFilename, Material* pxMaterial, TransformComponent& xTrans, Entity* xEntity);
		ModelComponent() = delete;
		~ModelComponent() {
			delete m_pxModel;
		}
		VCEModel* GetModel() const { return m_pxModel; }

		TransformComponent& GetTransformRef() { return m_xTransRef; }
	private:
		friend class ModelComponent;
		VCEModel* m_pxModel;
		std::string m_strFilename;

		TransformComponent& m_xTransRef;
		Entity& m_xParentEntity;
	};
}
