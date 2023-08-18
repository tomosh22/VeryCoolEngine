#pragma once

#include "VeryCoolEngine/Renderer/Buffer.h"
#include <glad/glad.h>
namespace VeryCoolEngine {


	class OpenGLVertexBuffer : public VertexBuffer{
	public:
		OpenGLVertexBuffer(void* verts, size_t size);
		~OpenGLVertexBuffer() override { glDeleteBuffers(1, &_id); };

		void UploadData() override;

		void Bind() const override;
		void Unbind() const override;

		void SetLayout(const BufferLayout& layout) override { _Layout = layout; }
		const BufferLayout& GetLayout() override { return _Layout; }
	private:
		GLuint _id;
	};

	class OpenGLIndexBuffer : public IndexBuffer {
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		~OpenGLIndexBuffer() override { glDeleteBuffers(1, &_id); };

		void UploadData() override;

		void Bind() const override;
		void Unbind() const override;
	private:
		GLuint _id;
	};
}


