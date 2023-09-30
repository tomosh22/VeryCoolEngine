#pragma once
#include "VeryCoolEngine/Renderer/Pipeline.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace VeryCoolEngine {
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(Shader* pxShader, BufferLayout xLayout, MeshTopolgy xTopology, const std::vector<ManagedUniformBuffer*>& apxUBOs, RenderPass* xRenderPass);

		vk::Pipeline m_xPipeline;
		vk::PipelineLayout m_xPipelineLayout;
	};

}
