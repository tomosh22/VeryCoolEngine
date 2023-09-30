#include "vcepch.h"
#include "Buffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"

namespace VeryCoolEngine {



	VertexBuffer* VertexBuffer::Create(void* verts, size_t size) {
#ifdef VCE_OPENGL
		return new OpenGLVertexBuffer(verts, size);
#endif
#ifdef VCE_VULKAN
		return new VulkanVertexBuffer(verts, size);
#endif
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count){
#ifdef VCE_OPENGL
		return new OpenGLIndexBuffer(indices, count);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
	
}