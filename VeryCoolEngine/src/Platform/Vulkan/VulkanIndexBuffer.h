#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace VeryCoolEngine {

	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer() = default;
		VulkanIndexBuffer(const VulkanIndexBuffer& other) = delete;
		~VulkanIndexBuffer() override {
			VCE_DELETE(m_pxIndexBuffer);
		}

		VulkanIndexBuffer(void* m_puIndices, size_t size);

		void UploadData() override;
		void Bind() const override;
		void Unbind() const override;

		
		VulkanBuffer* m_pxIndexBuffer;
	};

}


