#pragma once
#include "VeryCoolEngine/Renderer/Mesh.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"

namespace VeryCoolEngine {

	class VulkanMesh : public Mesh
	{
	public:
		virtual void PlatformInit() override;
		virtual void SetVertexArray(VertexArray* vertexArray) override;
		virtual VertexBuffer* CreateInstancedVertexBuffer() override;

		std::vector<vk::VertexInputBindingDescription> m_axBindDescs;
		std::vector<vk::VertexInputAttributeDescription> m_axAttrDescs;

		static vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType t);

		VulkanVertexBuffer* m_pxVertexBuffer;
		VulkanIndexBuffer* m_pxIndexBuffer;
		VulkanVertexBuffer* m_pxInstanceBuffer = nullptr;
	private:
		
	};

}