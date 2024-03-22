#include "vcepch.h"
#include "Buffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"

namespace VeryCoolEngine {



	VertexBuffer* VertexBuffer::Create(void* pData, size_t size, bool bDeviceLocal /*= true*/) {
#ifdef VCE_OPENGL
		return new OpenGLVertexBuffer(verts, size);
#endif
#ifdef VCE_VULKAN
		return new VulkanVertexBuffer(pData, size, bDeviceLocal);
#endif
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* m_puIndices, uint32_t count){
#ifdef VCE_OPENGL
		return new OpenGLIndexBuffer(indices, count);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}

	Buffer* Buffer::Create(uint64_t uSize) {
#ifdef VCE_OPENGL
		return new OpenGLIndexBuffer(indices, count);
#endif
#ifdef VCE_VULKAN
		VCE_INFO("implement me");
		return nullptr;

#endif
	}

	Buffer* Buffer::CreateStaging(uint64_t uSize) {
#ifdef VCE_OPENGL
		return new OpenGLIndexBuffer(indices, count);
#endif
#ifdef VCE_VULKAN
		return new VulkanBuffer(uSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

#endif
	}
	
}