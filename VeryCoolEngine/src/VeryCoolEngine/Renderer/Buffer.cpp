#include "vcepch.h"
#include "Buffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"

namespace VeryCoolEngine {



	VertexBuffer* VertexBuffer::Create(void* m_pVerts, size_t size) {
#ifdef VCE_OPENGL
		return new OpenGLVertexBuffer(verts, size);
#endif
#ifdef VCE_VULKAN
		return new VulkanVertexBuffer(m_pVerts, size);
#endif
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* m_puIndices, uint32_t count){
#ifdef VCE_OPENGL
		return new OpenGLIndexBuffer(indices, count);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
	
}