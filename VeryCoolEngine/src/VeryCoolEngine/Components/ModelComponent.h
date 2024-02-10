#pragma once
#include "VeryCoolEngine/Renderer/Model.h"
#include "TransformComponent.h"

namespace VeryCoolEngine {
	class Material;
	class ModelComponent
	{
	public:
		ModelComponent(std::string strFilename, TransformComponent& xTrans);
		ModelComponent(std::string strFilename, Material* pxMaterial, TransformComponent& xTrans);
		ModelComponent() = delete;
		~ModelComponent() {
			delete m_pxModel;
		}
		VCEModel* GetModel() const { return m_pxModel; }

		const TransformComponent& GetTransformRef() const { return m_xTransRef; }
	private:
		friend class ModelComponent;
		VCEModel* m_pxModel;
		std::string m_strFilename;

		TransformComponent& m_xTransRef;
	};
}
