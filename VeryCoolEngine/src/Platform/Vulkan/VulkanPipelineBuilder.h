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

		std::vector<vk::DescriptorSetLayout> m_axBufferDescLayouts;
		std::vector<vk::DescriptorSet> m_axBufferDescSets;





		std::vector<vk::DescriptorSetLayout> m_axTexDescLayouts;
		std::vector<vk::DescriptorSet> m_axTexDescSets;

		bool bUsePushConstants = false;//#TODO expand on this, currently just use model matrix

		std::string m_strName;

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
		VulkanPipelineBuilder& WithColourFormats(const std::vector<ColourFormat>& formats);
		VulkanPipelineBuilder& WithTesselation();

		//VulkanPipelineBuilder& WithDescriptorBuffers();

		VulkanPipeline*	Build(vk::PipelineCache cache = {});

		static VulkanPipeline* FromSpecification(const PipelineSpecification& spec);

	protected:
		struct DescriptorThings {
			std::vector<vk::DescriptorSetLayout> xBufferLayouts;
			std::vector<vk::DescriptorSet> xBufferSets;
			std::vector<vk::DescriptorSetLayout> xTexLayouts;
			std::vector<vk::DescriptorSet> xTexSets;
		};
		static DescriptorThings HandleDescriptorsOld(const PipelineSpecification& spec, VulkanPipelineBuilder& xBuilder);
		static DescriptorThings HandleDescriptorsNew(const PipelineSpecification& spec, VulkanPipelineBuilder& xBuilder);

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

		std::vector<ColourFormat> allColourRenderingFormats;
		vk::Format depthRenderingFormat;
		vk::Format stencilRenderingFormat;

		bool useTesselation;
		vk::PipelineTessellationStateCreateInfo tesselationCreate;

		std::string debugName;
	};
}