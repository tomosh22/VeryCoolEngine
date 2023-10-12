#include "vcepch.h"
#include "OpenGLBuffer.h"

namespace VeryCoolEngine {
	OpenGLVertexBuffer::OpenGLVertexBuffer(void* m_pVerts, size_t size) {
		glGenBuffers(1, &_id);
		glBindBuffer(GL_ARRAY_BUFFER, _id);

		glBufferData(GL_ARRAY_BUFFER, size, m_pVerts, GL_STATIC_DRAW);
	}
	void OpenGLVertexBuffer::UploadData(){
	}
	void OpenGLVertexBuffer::Bind() const{
		glBindBuffer(GL_ARRAY_BUFFER, _id);
	}
	void OpenGLVertexBuffer::Unbind() const	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}




	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* m_puIndices, uint32_t count) {
		_Count = count;
		glGenBuffers(1, &_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), m_puIndices, GL_STATIC_DRAW);
	}
	void OpenGLIndexBuffer::UploadData(){
	}
	void OpenGLIndexBuffer::Bind() const{
	}
	void OpenGLIndexBuffer::Unbind() const{
	}
}