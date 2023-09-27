#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace VeryCoolEngine {
	
	class VulkanVertexBuffer : VertexBuffer
	{
	public:
		VulkanVertexBuffer(void* verts, size_t size);
		VulkanVertexBuffer() {
			VulkanRenderer::GetInstance()->m_device.destroyBuffer(m_xVertexBuffer);
			VulkanRenderer::GetInstance()->m_device.freeMemory(m_xDeviceMem);
		}
		void UploadData() override;
		void Bind() const override {}
		void Unbind() const override {}

		void SetLayout(const BufferLayout& layout) override { _Layout = layout; }
		const BufferLayout& GetLayout() override { return _Layout; }

		vk::Buffer m_xVertexBuffer;
		vk::DeviceMemory m_xDeviceMem;
	private:
		
	};
}

