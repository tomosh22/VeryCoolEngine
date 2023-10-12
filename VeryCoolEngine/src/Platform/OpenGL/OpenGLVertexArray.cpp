#include "vcepch.h"
#include "OpenGLVertexArray.h"
#include "OpenGLBuffer.h"


namespace VeryCoolEngine {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType t) {
		switch (t) {
		case ShaderDataType::Float: return GL_FLOAT;
		case ShaderDataType::Float2: return GL_FLOAT;
		case ShaderDataType::Float3: return GL_FLOAT;
		case ShaderDataType::Float4: return GL_FLOAT;
		case ShaderDataType::Int: return GL_INT;
		case ShaderDataType::Int2: return GL_INT;
		case ShaderDataType::Int3: return GL_INT;
		case ShaderDataType::Int4: return GL_INT;
		case ShaderDataType::UInt: return GL_UNSIGNED_INT;
		case ShaderDataType::UInt2: return GL_UNSIGNED_INT;
		case ShaderDataType::UInt3: return GL_UNSIGNED_INT;
		case ShaderDataType::UInt4: return GL_UNSIGNED_INT;
		case ShaderDataType::Mat3: return GL_FLOAT;
		case ShaderDataType::Mat4: return GL_FLOAT;
		case ShaderDataType::Bool: return GL_BOOL;
		}
	}


	OpenGLVertexArray::OpenGLVertexArray() {
		glGenVertexArrays(1, &_id);
	}
	void OpenGLVertexArray::Bind() const {
		glBindVertexArray(_id);
	}

	void OpenGLVertexArray::Unbind() const {
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer, bool instanced) {
		glBindVertexArray(_id);
		vertexBuffer->Bind();
		for (const BufferElement& element : vertexBuffer->GetLayout()) {
			
			//#todo re add support for matrices
			VCE_ASSERT(element._Type != ShaderDataType::Mat3 && element._Type != ShaderDataType::Mat4,
				"matricesnot supported, too big");
				
			glEnableVertexAttribArray(_numVertAttribs);
			if (element._Type >= ShaderDataType::Int && element._Type <= ShaderDataType::UInt4) { //#todowrite utility function for this (this will be true if the data type is made of integers)
				glVertexAttribIPointer(_numVertAttribs,
					ShaderDataTypeNumElements(element._Type),
					ShaderDataTypeToOpenGLBaseType(element._Type),
					vertexBuffer->GetLayout().GetStride(),
					(const void*)element._Offset);
			}
			else {
				glVertexAttribPointer(_numVertAttribs,
					ShaderDataTypeNumElements(element._Type),
					ShaderDataTypeToOpenGLBaseType(element._Type),
					element._Normalized ? GL_TRUE : GL_FALSE,
					vertexBuffer->GetLayout().GetStride(),
					(const void*)element._Offset);
			}
			glVertexAttribDivisor(_numVertAttribs++, element.m_uDivisor);
				
			
		}
		_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::DisableVertexBuffer(VertexBuffer* vertexBuffer)
	{
		glBindVertexArray(_id);
		vertexBuffer->Bind();
		for (const BufferElement& element : vertexBuffer->GetLayout()) {
			//#todo this will work fine for now as we only have one vbo
			//that will be en/disabled, will need to store attribute ids
			//per vertex buffer in future
			glVertexAttribPointer(_numVertAttribs,
				0,
				0,
				0,
				0,
				0);
			glDisableVertexAttribArray(_numVertAttribs--);
		}
		//same for this
		vertexBuffer->Unbind();
		VCE_ASSERT(_VertexBuffers.size() == 2, "vertex buffer has come out of nowhere");
		_VertexBuffers.pop_back();
		glBindVertexArray(0);
		glDeleteBuffers(1, &(((OpenGLVertexBuffer*)vertexBuffer)->_id));
	}

	void OpenGLVertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {
		glBindVertexArray(_id);
		indexBuffer->Bind();
		_pIndexBuffer = indexBuffer;
	}

}
