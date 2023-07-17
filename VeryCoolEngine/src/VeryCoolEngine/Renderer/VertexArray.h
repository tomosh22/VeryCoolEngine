#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
namespace VeryCoolEngine {
	class VertexArray{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(VertexBuffer* vertexBuffer) = 0;
		virtual void SetIndexBuffer(IndexBuffer* indexBuffer) = 0;

		std::vector<VertexBuffer*>& GetVertexBuffers() { return _VertexBuffers; }
		IndexBuffer* GetIndexBuffer() const { return _pIndexBuffer; }

		static VertexArray* Create();
	protected:
		std::vector<VertexBuffer*> _VertexBuffers;
		IndexBuffer* _pIndexBuffer;
	};
}


