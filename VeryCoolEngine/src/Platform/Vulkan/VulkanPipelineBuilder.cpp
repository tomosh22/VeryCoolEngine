/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "vcepch.h"
#include "VulkanPipelineBuilder.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanManagedUniformBuffer.h"




namespace VeryCoolEngine {

	void VulkanPipeline::BindDescriptorSets(vk::CommandBuffer& xCmd, const std::vector<vk::DescriptorSet>& axSets, vk::PipelineBindPoint eBindPoint, uint32_t ufirstSet) const {
		xCmd.bindDescriptorSets(eBindPoint, m_xPipelineLayout, 0, axSets.size(), axSets.data(), 0, nullptr);
	}

	VulkanPipelineBuilder::VulkanPipelineBuilder(const std::string& pipeName) {
		dynamicStateEnables[0] = vk::DynamicState::eViewport;
		dynamicStateEnables[1] = vk::DynamicState::eScissor;

		dynamicCreate.setDynamicStateCount(2);
		dynamicCreate.setPDynamicStates(dynamicStateEnables);

		sampleCreate.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		viewportCreate.setViewportCount(1);
		viewportCreate.setScissorCount(1);

		pipelineCreate.setPViewportState(&viewportCreate);

		depthStencilCreate.setDepthCompareOp(vk::CompareOp::eAlways)
			.setDepthTestEnable(false)
			.setDepthWriteEnable(false)
			.setStencilTestEnable(false)
			.setDepthBoundsTestEnable(false);

		depthRenderingFormat = vk::Format::eUndefined;
		stencilRenderingFormat = vk::Format::eUndefined;

		rasterCreate.setCullMode(vk::CullModeFlagBits::eNone)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setLineWidth(1.0f);

		debugName = pipeName;

		inputAsmCreate.setTopology(vk::PrimitiveTopology::eTriangleList);
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithDepthState(vk::CompareOp op, bool depthEnabled, bool writeEnabled, bool stencilEnabled) {
		depthStencilCreate.setDepthCompareOp(op)
			.setDepthTestEnable(depthEnabled)
			.setDepthWriteEnable(writeEnabled)
			.setStencilTestEnable(stencilEnabled);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithBlendState(vk::BlendFactor srcState, vk::BlendFactor dstState, bool isEnabled) {
		vk::PipelineColorBlendAttachmentState pipeBlend;

		pipeBlend.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(isEnabled)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			.setColorBlendOp(vk::BlendOp::eAdd)

			.setSrcAlphaBlendFactor(srcState)
			.setSrcColorBlendFactor(srcState)

			.setDstAlphaBlendFactor(dstState)
			.setDstColorBlendFactor(dstState);

		blendAttachStates.emplace_back(pipeBlend);

		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithRaster(vk::CullModeFlagBits cullMode, vk::PolygonMode polyMode) {
		rasterCreate.setCullMode(cullMode).setPolygonMode(polyMode);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithVertexInputState(const vk::PipelineVertexInputStateCreateInfo& spec) {
		vertexCreate = spec;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithTopology(vk::PrimitiveTopology topology) {
		inputAsmCreate.setTopology(topology);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithShader(const VulkanShader& shader) {
		shader.FillShaderStageCreateInfo(pipelineCreate);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithLayout(vk::PipelineLayout layout) {
		this->layout = layout;
		pipelineCreate.setLayout(layout);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithPushConstant(vk::ShaderStageFlags flags, uint32_t offset, uint32_t size) {
		allPushConstants.emplace_back(vk::PushConstantRange(flags, offset, size));
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithPass(vk::RenderPass& renderPass) {
		pipelineCreate.setRenderPass(renderPass);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithDepthStencilFormat(vk::Format depthFormat) {
		depthRenderingFormat = depthFormat;
		stencilRenderingFormat = depthFormat;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithDepthFormat(vk::Format depthFormat) {
		depthRenderingFormat = depthFormat;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithColourFormats(const std::vector<vk::Format>& formats) {
		allColourRenderingFormats = formats;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithDescriptorSetLayout(uint32_t slot, vk::DescriptorSetLayout layout) {
		assert(slot < 32);
		while (allLayouts.size() <= slot) {
			allLayouts.push_back(vk::DescriptorSetLayout());
		}
		allLayouts[slot] = layout;
		return *this;
	}

	//VulkanPipelineBuilder& VulkanPipelineBuilder::WithDescriptorBuffers() {
	//	pipelineCreate.flags |= vk::PipelineCreateFlagBits::eDescriptorBufferEXT;
	//	return *this;
	//}

	VulkanPipeline*	VulkanPipelineBuilder::Build(vk::PipelineCache cache) {
		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		vk::PipelineLayoutCreateInfo pipeLayoutCreate = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(allLayouts)
			.setPushConstantRanges(allPushConstants);

		if (blendAttachStates.empty()) {
			if (!allColourRenderingFormats.empty()) {
				for (int i = 0; i < allColourRenderingFormats.size(); ++i) {
					WithBlendState(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, false);
				}
			}
			else {
				WithBlendState(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, false);
			}
		}

		//Patch any invalid descriptors to be empty
		/*vk::DescriptorSetLayout nullLayout = Vulkan::nullDescriptors[device];
		for (int i = 0; i < allLayouts.size(); ++i) {
			if (!allLayouts[i]) {
				allLayouts[i] = nullLayout;
			}
		}*/

		blendCreate.setAttachments(blendAttachStates);
		blendCreate.setBlendConstants({ 1.0f, 1.0f, 1.0f, 1.0f });

		VulkanPipeline* output = new VulkanPipeline();
		int uShouldReturn = 0;

		output->m_xPipelineLayout = xDevice.createPipelineLayout(pipeLayoutCreate);



		pipelineCreate.setPColorBlendState(&blendCreate)
			.setPDepthStencilState(&depthStencilCreate)
			.setPDynamicState(&dynamicCreate)
			.setPInputAssemblyState(&inputAsmCreate)
			.setPMultisampleState(&sampleCreate)
			.setPRasterizationState(&rasterCreate)
			.setLayout(output->m_xPipelineLayout)
			.setPVertexInputState(&vertexCreate);

		//We must be using dynamic rendering, better set it up!
		/*vk::PipelineRenderingCreateInfoKHR			renderingCreate;
		if (!allColourRenderingFormats.empty() || depthRenderingFormat != vk::Format::eUndefined) {
			renderingCreate.depthAttachmentFormat = depthRenderingFormat;
			renderingCreate.stencilAttachmentFormat = stencilRenderingFormat;

			renderingCreate.colorAttachmentCount = (uint32_t)allColourRenderingFormats.size();
			renderingCreate.pColorAttachmentFormats = allColourRenderingFormats.data();

			pipelineCreate.pNext = &renderingCreate;
		}*/
		output->m_xPipeline = xDevice.createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreate).value;

		return output;
		
	}
	VulkanPipeline* VulkanPipelineBuilder::FromSpecification(const PipelineSpecification& spec)
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		Application* app = Application::GetInstance();
		//#TODO utility function
		vk::PrimitiveTopology eTopology;
		switch (spec.m_pxExampleMesh->m_eTopolgy) {
		case(MeshTopolgy::Triangles):
			eTopology = vk::PrimitiveTopology::eTriangleList;
			break;
		default:
			VCE_ASSERT(false, "Invalid topolgy");
			break;
		
		}
		VulkanPipelineBuilder xBuilder = VulkanPipelineBuilder("Geometry Pipeline");
		xBuilder = xBuilder.WithVertexInputState(dynamic_cast<VulkanMesh*>(spec.m_pxExampleMesh)->m_xVertexInputState);
		xBuilder = xBuilder.WithTopology(eTopology);
		xBuilder = xBuilder.WithShader(*dynamic_cast<VulkanShader*>(dynamic_cast<VulkanMesh*>(spec.m_pxExampleMesh)->GetShader()));
		xBuilder = xBuilder.WithBlendState(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, false);
		xBuilder = xBuilder.WithDepthState(vk::CompareOp::eGreaterOrEqual, spec.m_bDepthTestEnabled, spec.m_bDepthWriteEnabled, false);
		xBuilder = xBuilder.WithColourFormats({ vk::Format::eB8G8R8A8Srgb });
		xBuilder = xBuilder.WithDepthFormat(vk::Format::eD32Sfloat);
			xBuilder = xBuilder.WithPass(dynamic_cast<VulkanRenderPass*>(*spec.m_pxRenderPass)->m_xRenderPass);

		
		std::vector<vk::DescriptorSetLayout> axLayouts;
		std::vector<vk::DescriptorSet> axSets;
		for (DescriptorSpecification spec : spec.m_axDescriptors) {
			VCE_ASSERT(spec.m_aeSamplerStages.size() == 0 || spec.m_aeUniformBufferStages.size() == 0, "Need to rework this sytem")
			axLayouts.emplace_back(VulkanDescriptorSetLayoutBuilder::FromSpecification(spec));
			axSets.emplace_back(pxRenderer->CreateDescriptorSet(axLayouts.back(), pxRenderer->GetDescriptorPool()));

			uint32_t uTexBinding = 0;
			for (auto& [ppxTexture, eStage] : spec.m_aeSamplerStages) {
				//VCE_ASSERT((*ppxTexture)->m_bInitialised, "Texture not initialised");
				pxRenderer->UpdateImageDescriptor(axSets.back(), uTexBinding++, 0, dynamic_cast<VulkanTexture2D*>(*ppxTexture)->m_xImageView, dynamic_cast<VulkanTexture2D*>(*ppxTexture)->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);
			}

			for(auto& [ppxUBO, eStage] : spec.m_aeUniformBufferStages) {
				for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
				{
					pxRenderer->UpdateBufferDescriptor(axSets.back(), dynamic_cast<VulkanManagedUniformBuffer*>(*ppxUBO)->ppBuffers[i], 0, vk::DescriptorType::eUniformBuffer, 0);
				}
			}
		}

		for (uint32_t i = 0; i < axLayouts.size(); i++) {
			xBuilder = xBuilder.WithDescriptorSetLayout(i, axLayouts.at(i));
		}

		VulkanPipeline* xPipeline = xBuilder.Build();
		xPipeline->m_axDescLayouts = axLayouts;
		xPipeline->m_axDescSets = axSets;
		xPipeline->m_strName = spec.m_strName;

		return xPipeline;
	}
}