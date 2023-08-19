#pragma once
#include "VertexArray.h"
#include "Renderer.h"

namespace VeryCoolEngine {
	class RenderCommand{
	public:
		static void SetClearColor(const glm::vec4 color) {
			_spRenderer->SetClearColor(color);
		}
		static void Clear() {
			_spRenderer->Clear();
		}
		static void DrawIndexed(VertexArray* vertexArray) {
			_spRenderer->DrawIndexed(vertexArray);
		}
		static void DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count) {
			_spRenderer->DrawIndexedInstanced(vertexArray, count);
		}
	private:
		static Renderer* _spRenderer;
	};

}

