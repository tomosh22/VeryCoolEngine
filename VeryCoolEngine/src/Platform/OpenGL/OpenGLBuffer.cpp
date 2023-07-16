#include "vcepch.h"
#include "OpenGLBuffer.h"

namespace VeryCoolEngine {
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* verts, size_t size) {
		glGenBuffers(1, &_id);
		glBindBuffer(GL_ARRAY_BUFFER, _id);

		glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);
	}
	void OpenGLVertexBuffer::UploadData(){
	}
	void OpenGLVertexBuffer::Bind() const{
	}
	void OpenGLVertexBuffer::Unbind() const	{
	}




	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) {
		_Count = count;
		glGenBuffers(1, &_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}
	void OpenGLIndexBuffer::UploadData(){
	}
	void OpenGLIndexBuffer::Bind() const{
	}
	void OpenGLIndexBuffer::Unbind() const{
	}
}