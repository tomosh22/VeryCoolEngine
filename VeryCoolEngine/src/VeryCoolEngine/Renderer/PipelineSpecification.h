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

	enum ShaderStage : uint8_t {
		ShaderStageNone = 0, ShaderStageVertex = 1, ShaderStageFragment = 2, ShaderStageVertexAndFragment = 3 //#TODO how do you do bitwise operations on an enum?????
	};
	

	struct DescriptorSpecification {
		std::vector<ShaderStage> m_aeUniformBufferStages;
		std::vector<ShaderStage> m_aeSamplerStages;
	};

	struct PipelineSpecification {
		std::string m_strName;
		Mesh* m_pxExampleMesh;
		BlendFactor m_eSrcBlendFactor;
		BlendFactor m_eDstBlendFactor;
		DepthCompareFunc m_eDepthCompareFunc;
		ColourFormat m_eColourFormat;
		DepthFormat m_eDepthFormat;
		std::vector<DescriptorSpecification> m_axDescriptors;
		RenderPass** m_pxRenderPass;
	};
	
	
}