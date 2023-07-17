#pragma once
#include "VertexArray.h"
#include "Renderer.h"

namespace VeryCoolEngine {
	class RenderCommand{
	public:
		static void SetClearColor(const glm::vec4 color) {
			Renderer::_spRenderer->SetClearColor(color);
		}
		static void Clear() {
			Renderer::_spRenderer->Clear();
		}
		static void DrawIndexed(VertexArray* vertexArray) {
			Renderer::_spRenderer->DrawIndexed(vertexArray);
		}
	private:
	};

}

