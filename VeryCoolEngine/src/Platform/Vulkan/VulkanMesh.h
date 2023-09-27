#pragma once
#include "VeryCoolEngine/Renderer/Mesh.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include "Platform/Vulkan/VulkanVertexBuffer.h"

namespace VeryCoolEngine {

	class VulkanMesh : public Mesh
	{
	public:
		virtual void PlatformInit() override;
		virtual void SetVertexArray(VertexArray* vertexArray) override;
		virtual VertexBuffer* CreateInstancedVertexBuffer() override;

		std::vector<vk::VertexInputBindingDescription> m_axBindDescs;
		std::vector<vk::VertexInputAttributeDescription> m_axAttrDescs;

		VulkanVertexBuffer* m_pxVertexBuffer;
	private:
		
	};

}