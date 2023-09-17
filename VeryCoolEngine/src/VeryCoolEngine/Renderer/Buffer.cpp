#include "vcepch.h"
#include "Buffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace VeryCoolEngine {



	VertexBuffer* VertexBuffer::Create(void* verts, size_t size) {
#ifdef VCE_OPENGL
		return new OpenGLVertexBuffer(verts, size);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count){
#ifdef VCE_OPENGL
		return new OpenGLIndexBuffer(indices, count);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
	
}