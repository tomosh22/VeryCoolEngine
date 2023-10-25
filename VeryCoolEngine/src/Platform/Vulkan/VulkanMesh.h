#pragma once
#include "VeryCoolEngine/Renderer/Mesh.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"

namespace VeryCoolEngine {

	class VulkanMesh : public Mesh
	{
	public:
		virtual void PlatformInit() override;
		virtual void SetVertexArray(VertexArray* vertexArray) override;
		virtual VertexBuffer* CreateInstancedVertexBuffer() override;

		void BindToCmdBuffer(vk::CommandBuffer& xCmdBuffer) const;

		std::vector<vk::VertexInputBindingDescription> m_axBindDescs;
		std::vector<vk::VertexInputAttributeDescription> m_axAttrDescs;

		static vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType t);

		VulkanVertexBuffer* m_pxVertexBuffer;
		VulkanIndexBuffer* m_pxIndexBuffer;
		VulkanVertexBuffer* m_pxInstanceBuffer = nullptr;

		vk::PipelineVertexInputStateCreateInfo m_xVertexInputState;

		
		vk::DescriptorSet m_xTexDescSet;
		vk::DescriptorSetLayout m_xTexDescSetLayout;
	private:
		
	};

}