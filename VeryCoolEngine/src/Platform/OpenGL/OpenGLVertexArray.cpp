#include "vcepch.h"
#include "OpenGLVertexArray.h"


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
			//should probably tidy this up so there's no need to branch here, problem is stride is used for non instanced buffers and not for instanced buffers, and offset works differently
			if (instanced) {
				//can only fit 16 bytes in one attribute slot, anything bigger than this will occupy multiple slots
				for (size_t i = 0; i <= ShaderDataTypeSize(element._Type) / 16 || i == 0; i++)
				{
					glEnableVertexAttribArray(_numVertAttribs);
					if (element._Type >= ShaderDataType::Int && element._Type <= ShaderDataType::UInt4) { //#todo write utlity function for this (this will be true if the data type is made of integers)
						glVertexAttribIPointer(_numVertAttribs,
							ShaderDataTypeNumElements(element._Type),
							ShaderDataTypeToOpenGLBaseType(element._Type),
							ShaderDataTypeSize(element._Type),
							(void*)(i * 16));
					}
					else {
						glVertexAttribPointer(_numVertAttribs,
							ShaderDataTypeNumElements(element._Type),
							ShaderDataTypeToOpenGLBaseType(element._Type),
							element._Normalized ? GL_TRUE : GL_FALSE,
							ShaderDataTypeSize(element._Type),
							(void*)(i * 16));
					}
					glVertexAttribDivisor(_numVertAttribs++, element._divisor);
				}
			}
			else {
				glEnableVertexAttribArray(_numVertAttribs);
				glVertexAttribPointer(_numVertAttribs,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element._Type),
					element._Normalized ? GL_TRUE : GL_FALSE,
					vertexBuffer->GetLayout().GetStride(),
					(const void*)element._Offset);
				_numVertAttribs++;
			}
		}
		_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {
		glBindVertexArray(_id);
		indexBuffer->Bind();
		_pIndexBuffer = indexBuffer;
	}

}
