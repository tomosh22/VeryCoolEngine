#include "vcepch.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanMesh.h"
#include "VulkanManagedUniformBuffer.h"
#include "VulkanRenderPass.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace VeryCoolEngine {
	VulkanPipeline::VulkanPipeline(Shader* pxShader, BufferLayout* xLayout, MeshTopolgy xTopology, std::vector<ManagedUniformBuffer**> apxUBOs, RenderPass** xRenderPass)
	{
		m_pxShader = pxShader;
		m_xVertexBufferLayout = xLayout;
		m_xTopology = xTopology;
		m_apxUBOs = apxUBOs;
		m_pxRenderPass = xRenderPass;
	}
	void VulkanPipeline::PlatformInit() {

		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		
		VulkanShader* pxVulkanShader = dynamic_cast<VulkanShader*>(m_pxShader);

		vk::ShaderModule vertShaderModule = pxVulkanShader->xVertShaderModule;
		vk::ShaderModule fragShaderModule = pxVulkanShader->xFragShaderModule;

		vk::PipelineShaderStageCreateInfo vertStageInfo{};
		vertStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertStageInfo.module = vertShaderModule;
		vertStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo fragStageInfo{};
		fragStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragStageInfo.module = fragShaderModule;
		fragStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo stages[] = { vertStageInfo,fragStageInfo };

#pragma region VertexBindings
		std::vector<vk::VertexInputBindingDescription> axBindDescs;
		std::vector<vk::VertexInputAttributeDescription> axAttrDescs;

		m_xVertexBufferLayout->CalculateOffsetsAndStrides();
		uint32_t uBindPoint = 0;
		uint32_t uTotalSize = 0;
		for (BufferElement& element : m_xVertexBufferLayout->GetElements()) {


			vk::VertexInputAttributeDescription xAttrDesc = vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(uBindPoint)
				.setOffset(element._Offset)
				.setFormat(VulkanMesh::ShaderDataTypeToVulkanFormat(element._Type));
			axAttrDescs.push_back(xAttrDesc);
			uBindPoint++;
			uTotalSize += element._Offset;
		}

		vk::VertexInputBindingDescription xBindDesc = vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(m_xVertexBufferLayout->_Stride)//I changed this from sizeof(float) * numFloats
			.setInputRate(vk::VertexInputRate::eVertex);
		axBindDescs.push_back(xBindDesc);
#pragma endregion


		Application* app = Application::GetInstance();
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->_pMesh);
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.vertexBindingDescriptionCount = pxVulkanMesh->m_axBindDescs.size();
		vertexInputInfo.pVertexBindingDescriptions = pxVulkanMesh->m_axBindDescs.data();
		vertexInputInfo.vertexAttributeDescriptionCount = pxVulkanMesh->m_axAttrDescs.size();
		vertexInputInfo.pVertexAttributeDescriptions = pxVulkanMesh->m_axAttrDescs.data();

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssembly.primitiveRestartEnable = VK_FALSE;




		vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		vk::PipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eCounterClockwise;//TODO change to ccw
		rasterizer.depthBiasEnable = VK_FALSE;

		vk::PipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

		//TODO depth testing

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_TRUE;

		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		vk::DescriptorSetLayout* axLayouts = new vk::DescriptorSetLayout[m_apxUBOs.size()];
		uint32_t uIndex = 0;
		for (ManagedUniformBuffer** ubo : m_apxUBOs) {
			VulkanManagedUniformBuffer* pxVulkanUBO = dynamic_cast< VulkanManagedUniformBuffer*>(*ubo);
			axLayouts[uIndex] = pxVulkanUBO->m_xDescriptorLayout;
		}

		vk::PipelineLayoutCreateInfo xPipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(m_apxUBOs.size())
			.setPSetLayouts(axLayouts);
		m_xPipelineLayout = xDevice.createPipelineLayout(xPipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = stages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_xPipelineLayout;
		pipelineInfo.renderPass = dynamic_cast<VulkanRenderPass*>(*m_pxRenderPass)->m_xRenderPass;
		pipelineInfo.subpass = 0;


		m_xPipeline = xDevice.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo).value;

		xDevice.destroyShaderModule(vertShaderModule, nullptr);
		xDevice.destroyShaderModule(fragShaderModule, nullptr);
	}

}