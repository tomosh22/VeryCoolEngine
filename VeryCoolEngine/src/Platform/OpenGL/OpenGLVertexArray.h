#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include <glad/glad.h>
namespace VeryCoolEngine {
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray() {
			for (VertexBuffer* b : _VertexBuffers) delete b;
			delete _pIndexBuffer;
			glDeleteVertexArrays(1, &_id);
		}

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(VertexBuffer* vertexBuffer, bool instanced = false) override;
		void DisableVertexBuffer(VertexBuffer* vertexBuffer) override;
		void SetIndexBuffer(IndexBuffer* indexBuffer) override;

	private:
		
		GLuint _id;
	};

}

