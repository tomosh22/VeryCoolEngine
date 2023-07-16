#include "vcepch.h"
#include "Buffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace VeryCoolEngine {

	VertexBuffer* VertexBuffer::Create(float* verts, size_t size) {
#ifdef VCE_OPENGL
		return new OpenGLVertexBuffer(verts, size);
#endif

	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count){
#ifdef VCE_OPENGL
		return new OpenGLIndexBuffer(indices, count);
#endif
	}
	
}