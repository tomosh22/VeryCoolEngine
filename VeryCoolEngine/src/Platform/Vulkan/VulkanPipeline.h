#pragma once
#include "VeryCoolEngine/Renderer/Pipeline.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "VulkanRenderer.h"

namespace VeryCoolEngine {
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(Shader* pxShader, BufferLayout* xLayout, MeshTopolgy xTopology, std::vector<ManagedUniformBuffer**> apxUBOs, std::vector<Texture2D**> apxTextures, RenderPass** xRenderPass);
		~VulkanPipeline() {

			VulkanRenderer::GetInstance()->GetDevice().destroyPipeline(m_xPipeline, nullptr);
			VulkanRenderer::GetInstance()->GetDevice().destroyPipelineLayout(m_xPipelineLayout, nullptr);
		}

		void PlatformInit() override;

		vk::Pipeline m_xPipeline;
		vk::PipelineLayout m_xPipelineLayout;

		vk::DescriptorSetLayout m_xDescriptorLayout;
		vk::DescriptorSet m_axDescriptorSets[MAX_FRAMES_IN_FLIGHT];
	};

}
