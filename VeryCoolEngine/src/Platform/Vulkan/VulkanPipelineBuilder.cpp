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
		xCmd.bindDescriptorSets(eBindPoint, m_xPipelineLayout, ufirstSet, axSets.size(), axSets.data(), 0, nullptr);
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

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithPushConstant(vk::ShaderStageFlags flags, uint32_t offset) {
		allPushConstants.emplace_back(vk::PushConstantRange(flags, offset, VulkanRenderer::GetInstance()->GetPhysicalDevice().getProperties().limits.maxPushConstantsSize));
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

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithColourFormats(const std::vector<ColourFormat>& formats) {
		allColourRenderingFormats = formats;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::WithTesselation()
	{
		useTesselation = true;
		inputAsmCreate.setTopology(vk::PrimitiveTopology::ePatchList);
		tesselationCreate.setPatchControlPoints(3);
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

		if (useTesselation)
			pipelineCreate = pipelineCreate.setPTessellationState(&tesselationCreate);

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

	vk::Format VceFormatToVKFormat(ColourFormat eFmt) {
		switch (eFmt) {
		case ColourFormat::BGRA8_sRGB:
			return vk::Format::eB8G8R8A8Srgb;
		case ColourFormat::BGRA8_Unorm:
			return vk::Format::eB8G8R8A8Unorm;
		default:
			VCE_ASSERT(false, "Unsupported format");
		}
	}

	vk::CompareOp VceCompareFuncToVkCompareFunc(DepthCompareFunc eFunc) {
		switch (eFunc) {
		case DepthCompareFunc::GreaterOrEqual:
			return vk::CompareOp::eGreaterOrEqual;
		case DepthCompareFunc::LessOrEqual:
			return vk::CompareOp::eLessOrEqual;
		case DepthCompareFunc::Never:
			return vk::CompareOp::eNever;
		case DepthCompareFunc::Always:
			return vk::CompareOp::eAlways;
		case DepthCompareFunc::Disabled:
			return vk::CompareOp::eAlways;
		default:
			VCE_ASSERT(false, "Unsupported blend factor");
		}
	}

	vk::BlendFactor VceBlendFactorToVKBlendFactor(BlendFactor eFactor) {
		switch (eFactor) {
		case BlendFactor::SrcAlpha:
			return vk::BlendFactor::eSrcAlpha;
		case BlendFactor::OneMinusSrcAlpha:
			return vk::BlendFactor::eOneMinusSrcAlpha;
		default:
			VCE_ASSERT(false, "Unsupported blend factor");
		}
	}


	VulkanPipelineBuilder::DescriptorThings VulkanPipelineBuilder::HandleDescriptorsNew(const PipelineSpecification& spec, VulkanPipelineBuilder& xBuilder)
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();

		uint32_t uSetIndex = 0;
		std::vector<vk::DescriptorSetLayout> xLayouts;
		std::vector<vk::DescriptorSet> xSets;

		//			  buffers    textures
		for (std::pair<uint32_t, uint32_t> xSetDesc : spec.m_xDescSetBindings) {
			VulkanDescriptorSetLayoutBuilder xDescBuilder = VulkanDescriptorSetLayoutBuilder().WithBindlessAccess();

			for (uint32_t i = 0; i < xSetDesc.first; i++)
				xDescBuilder = xDescBuilder.WithUniformBuffers(1);
			for (uint32_t i = 0; i < xSetDesc.second; i++)
				xDescBuilder = xDescBuilder.WithSamplers(1);

			vk::DescriptorSetLayout xLayout = xDescBuilder.Build(pxRenderer->GetDevice());

			xBuilder = xBuilder.WithDescriptorSetLayout(uSetIndex, xLayout);

			xLayouts.push_back(xLayout);

			xSets.push_back(pxRenderer->CreateDescriptorSet(xLayout, pxRenderer->GetDescriptorPool()));

			uSetIndex++;
		}

		return { xLayouts, xSets };
		
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

		VCE_ASSERT(spec.m_aeDstBlendFactors.size() == spec.m_aeSrcBlendFactors.size(), "Do I need to rework this?");

		VulkanPipelineBuilder xBuilder = VulkanPipelineBuilder(spec.m_strName.c_str());
		if (!spec.m_pxExampleMesh->m_bInitialised)
			spec.m_pxExampleMesh->PlatformInit();
		xBuilder = xBuilder.WithVertexInputState(dynamic_cast<VulkanMesh*>(spec.m_pxExampleMesh)->m_xVertexInputState);
		xBuilder = xBuilder.WithTopology(eTopology);
		if (!spec.m_pxShader->m_bInitialised)
			spec.m_pxShader->PlatformInit();
		xBuilder = xBuilder.WithShader(*dynamic_cast<VulkanShader*>(spec.m_pxShader));
		for (uint32_t i = 0; i < spec.m_aeDstBlendFactors.size(); i++)
			xBuilder = xBuilder.WithBlendState(VceBlendFactorToVKBlendFactor(spec.m_aeSrcBlendFactors[i]), VceBlendFactorToVKBlendFactor(spec.m_aeDstBlendFactors[i]), spec.m_abBlendStatesEnabled[i]);
		xBuilder = xBuilder.WithDepthState(VceCompareFuncToVkCompareFunc(spec.m_eDepthCompareFunc), spec.m_bDepthTestEnabled, spec.m_bDepthWriteEnabled, false);
		xBuilder = xBuilder.WithColourFormats(spec.m_aeColourFormats);
		xBuilder = xBuilder.WithDepthFormat(vk::Format::eD32Sfloat);
		xBuilder = xBuilder.WithPass(pxRenderer->m_xTargetSetupPasses.at(spec.m_strRenderPassName.c_str())->m_xRenderPass);

		DescriptorThings xDescThings = HandleDescriptorsNew(spec, xBuilder);

		if (spec.m_bUsePushConstants) {
			xBuilder = xBuilder.WithPushConstant(vk::ShaderStageFlagBits::eAll, 0);
		}

		if (spec.m_bUseTesselation)
			xBuilder = xBuilder.WithTesselation();

		VulkanPipeline* xPipeline = xBuilder.Build();
		xPipeline->m_axDescLayouts = xDescThings.xLayouts;
		for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			xPipeline->m_axDescSets[i] = xDescThings.xSets;
		xPipeline->m_strName = spec.m_strName;
		xPipeline->bUsePushConstants = spec.m_bUsePushConstants;
		

		return xPipeline;
	}
}