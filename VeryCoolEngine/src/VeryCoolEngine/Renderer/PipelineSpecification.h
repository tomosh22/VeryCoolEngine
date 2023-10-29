#pragma once
#include <vector>
#include <string>


namespace VeryCoolEngine {

	class Mesh;
	class Texture;

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
	

	struct BufferDescriptorSpecification {
		std::vector<std::pair<class ManagedUniformBuffer**, ShaderStage>> m_aeUniformBufferStages;
	};

	struct TextureDescriptorSpecification {
		std::vector<std::pair<Texture**, ShaderStage>> m_aeSamplerStages;
	};

	struct PipelineSpecification {
		std::string m_strName;
		Mesh* m_pxExampleMesh;
		BlendFactor m_eSrcBlendFactor;
		BlendFactor m_eDstBlendFactor;
		bool m_bDepthTestEnabled;
		bool m_bDepthWriteEnabled;
		DepthCompareFunc m_eDepthCompareFunc;
		ColourFormat m_eColourFormat;
		DepthFormat m_eDepthFormat;
		std::vector<BufferDescriptorSpecification> m_axBufferDescriptors;
		std::vector<TextureDescriptorSpecification> m_axTextureDescriptors;
		RenderPass** m_pxRenderPass;
		bool m_bUsePushConstants;//#TODO expand on this, currently just use model matrix
		bool m_bUseTesselation;
	};
	
	
}