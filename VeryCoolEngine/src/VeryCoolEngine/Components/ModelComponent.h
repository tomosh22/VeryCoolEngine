#pragma once
#include "VeryCoolEngine/Renderer/Model.h"
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/PlatformTypes.h"

namespace VeryCoolEngine {
	class Material;
	class ModelComponent
	{
	public:
		ModelComponent(std::string strFilename, TransformComponent& xTrans, Entity* xEntity);
		ModelComponent(std::string strFilename, GUID xMaterialGUID, TransformComponent& xTrans, Entity* xEntity);
		void Serialize(std::ofstream& xOut);
		ModelComponent() = delete;
		~ModelComponent() {
			delete m_pxModel;
		}
		VCEModel* GetModel() const { return m_pxModel; }

		TransformComponent& GetTransformRef() { return m_xTransRef; }
	private:
		VCEModel* m_pxModel;
		std::string m_strFilename;

		std::string m_strMaterialName;

		TransformComponent& m_xTransRef;
		Entity m_xParentEntity;
	};
}
