/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VeryCoolEngine/Renderer/PipelineSpecification.h"

namespace VeryCoolEngine {
	class VulkanShader;

	class VulkanPipeline {
	public:
		vk::Pipeline m_xPipeline;
		vk::PipelineLayout	m_xPipelineLayout;

		std::vector<vk::DescriptorSetLayout> m_axDescLayouts;
		std::vector<vk::DescriptorSet> m_axDescSets;

		void BindDescriptorSets(vk::CommandBuffer& xCmd, const std::vector<vk::DescriptorSet>& axSets, vk::PipelineBindPoint eBindPoint, uint32_t ufirstSet) const;
	};

	class VulkanPipelineBuilder	{
	public:
		VulkanPipelineBuilder(const std::string& debugName = "");
		~VulkanPipelineBuilder() {}

		VulkanPipelineBuilder& WithDepthState(vk::CompareOp op, bool depthEnabled, bool writeEnabled, bool stencilEnabled = false);

		VulkanPipelineBuilder& WithBlendState(vk::BlendFactor srcState, vk::BlendFactor dstState, bool enabled = true);

		VulkanPipelineBuilder& WithRaster(vk::CullModeFlagBits cullMode, vk::PolygonMode polyMode = vk::PolygonMode::eFill);

		VulkanPipelineBuilder& WithVertexInputState(const vk::PipelineVertexInputStateCreateInfo& spec);

		VulkanPipelineBuilder& WithTopology(vk::PrimitiveTopology topology);

		VulkanPipelineBuilder& WithShader(const VulkanShader& shader);

		VulkanPipelineBuilder& WithLayout(vk::PipelineLayout layout);

		VulkanPipelineBuilder& WithPushConstant(vk::ShaderStageFlags flags, uint32_t offset, uint32_t size);

		VulkanPipelineBuilder& WithDescriptorSetLayout(uint32_t slot, vk::DescriptorSetLayout layout);

		VulkanPipelineBuilder& WithPass(vk::RenderPass& renderPass);

		VulkanPipelineBuilder& WithDepthStencilFormat(vk::Format combinedFormat);
		VulkanPipelineBuilder& WithDepthFormat(vk::Format depthFormat);
		VulkanPipelineBuilder& WithColourFormats(const std::vector<vk::Format>& formats);

		//VulkanPipelineBuilder& WithDescriptorBuffers();

		VulkanPipeline*	Build(vk::PipelineCache cache = {});

		static VulkanPipeline* FromSpecification(const PipelineSpecification& spec);

	protected:
		vk::GraphicsPipelineCreateInfo				pipelineCreate;
		vk::PipelineCacheCreateInfo					cacheCreate;
		vk::PipelineInputAssemblyStateCreateInfo	inputAsmCreate;
		vk::PipelineRasterizationStateCreateInfo	rasterCreate;
		vk::PipelineColorBlendStateCreateInfo		blendCreate;
		vk::PipelineDepthStencilStateCreateInfo		depthStencilCreate;
		vk::PipelineViewportStateCreateInfo			viewportCreate;
		vk::PipelineMultisampleStateCreateInfo		sampleCreate;
		vk::PipelineDynamicStateCreateInfo			dynamicCreate;
		vk::PipelineVertexInputStateCreateInfo		vertexCreate;
		vk::PipelineLayout layout;

		std::vector< vk::PipelineColorBlendAttachmentState>			blendAttachStates;

		vk::DynamicState dynamicStateEnables[2];

		std::vector< vk::DescriptorSetLayout> allLayouts;
		std::vector< vk::PushConstantRange> allPushConstants;

		std::vector<vk::Format> allColourRenderingFormats;
		vk::Format depthRenderingFormat;
		vk::Format stencilRenderingFormat;

		std::string debugName;
	};
}