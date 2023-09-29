#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace VeryCoolEngine {
	
	class VulkanVertexBuffer : VertexBuffer
	{
	public:
		VulkanVertexBuffer(void* verts, size_t size);
		VulkanVertexBuffer() = default;
		void UploadData() override;
		void Bind() const override {}
		void Unbind() const override {}

		void SetLayout(const BufferLayout& layout) override { _Layout = layout; }
		const BufferLayout& GetLayout() override { return _Layout; }

		VulkanBuffer* m_pxVertexBuffer;
	private:
		
	};
}

