#pragma once
#include "Mesh.h"
#include <vector>
#include <string>


namespace VeryCoolEngine {

	enum class BlendFactor {
		Disabled, SrcAlpha, OneMinusSrcAlpha
	};

	enum class DepthCompareFunc {
		Disabled, GreaterOrEqual
	};

	enum class ColourFormat {
		BGRA8_sRGB
	};

	enum class DepthFormat {
		D32_SFloat
	};

	class DescriptorSetLayoutBase;
	class RenderPass;

	struct PipelineSpecification {
		std::string m_strName;
		Mesh* m_pxExampleMesh;
		BlendFactor m_eSrcBlendFactor;
		BlendFactor m_eDstBlendFactor;
		DepthCompareFunc m_eDepthCompareFunc;
		ColourFormat m_eColourFormat;
		DepthFormat m_eDepthFormat;
		std::vector<vk::DescriptorSetLayout*> m_aDescSetLayouts;
		RenderPass** m_pxRenderPass;
	};
}