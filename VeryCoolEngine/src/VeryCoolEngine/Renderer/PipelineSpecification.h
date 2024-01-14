#pragma once
#include <vector>
#include <string>


namespace VeryCoolEngine {

	class Mesh;
	class Shader;
	class Texture;

	enum class BlendFactor {
		Disabled, SrcAlpha, OneMinusSrcAlpha
	};

	enum class DepthCompareFunc {
		Disabled, LessOrEqual, GreaterOrEqual, Never, Always
	};

	enum class ColourFormat {
		BGRA8_sRGB,
		BGRA8_Unorm,
	};

	enum class DepthFormat {
		D32_SFloat
	};

	class DescriptorSetLayoutBase;
	class RenderPass;

	enum ShaderStage : uint8_t {
		ShaderStageNone = 0, ShaderStageVertex = 1, ShaderStageFragment = 2, ShaderStageVertexAndFragment = 3 //#TODO how do you do bitwise operations on an enum?????
	};
	
	struct TextureDescriptorSpecification {
		std::vector<std::pair<Texture**, ShaderStage>> m_aeSamplerStages;
		bool m_bJustFragment = false;
		bool m_bBindless = true;
	};

	struct PipelineSpecification {
		std::string m_strName;
		Mesh* m_pxExampleMesh;
		Shader* m_pxShader;

		//#TODO these need to be combined, this is disgusting
		std::vector<BlendFactor> m_aeSrcBlendFactors;
		std::vector<BlendFactor> m_aeDstBlendFactors;
		std::vector<bool> m_abBlendStatesEnabled;

		bool m_bDepthTestEnabled;
		bool m_bDepthWriteEnabled;
		DepthCompareFunc m_eDepthCompareFunc;
		std::vector<ColourFormat> m_aeColourFormats;
		DepthFormat m_eDepthFormat;
		RenderPass** m_pxRenderPass;
		bool m_bUsePushConstants;
		bool m_bUseTesselation;

		//					  buffers    textures
		std::vector<std::pair<uint32_t, uint32_t>> m_xDescSetBindings;
	};
	
	
}