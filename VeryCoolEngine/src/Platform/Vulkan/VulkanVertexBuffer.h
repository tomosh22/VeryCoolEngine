#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace VeryCoolEngine {
	
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(void* verts, size_t size);
		VulkanVertexBuffer() = default;
		~VulkanVertexBuffer() { delete m_pxVertexBuffer; }
		void UploadData() override;
		void Bind() const override {}
		void Unbind() const override {}

		void SetLayout(const BufferLayout& layout) override { _Layout = layout; }
		const BufferLayout& GetLayout() override { return _Layout; }

		VulkanBuffer* m_pxVertexBuffer;
	private:
		
	};
}

