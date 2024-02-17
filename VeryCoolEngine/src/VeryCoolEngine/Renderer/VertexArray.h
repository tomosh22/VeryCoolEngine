#pragma once
#include "VeryCoolEngine/Renderer/Buffer.h"
namespace VeryCoolEngine {
	class VertexArray{
	public:
		virtual ~VertexArray() {
			delete _pIndexBuffer;
		};

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(VertexBuffer* vertexBuffer, bool instanced = false) = 0;
		virtual void DisableVertexBuffer(VertexBuffer* vertexBuffer) = 0;
		virtual void SetIndexBuffer(IndexBuffer* indexBuffer) = 0;

		std::vector<VertexBuffer*>& GetVertexBuffers() { return _VertexBuffers; }
		IndexBuffer* GetIndexBuffer() const { return _pIndexBuffer; }

		static VertexArray* Create();
		unsigned int _numVertAttribs = 0;
		std::vector<VertexBuffer*> _VertexBuffers;
	protected:
		IndexBuffer* _pIndexBuffer;
	};
}


