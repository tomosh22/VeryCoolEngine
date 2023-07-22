#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"

namespace VeryCoolEngine {
	class Renderer
	{
	public:
		virtual void SetClearColor(const glm::vec4 color) = 0;
		virtual void Clear() = 0;
		
		virtual void BeginScene() = 0;
		virtual void EndScene() = 0;
		static void Submit(VertexArray* vertexArray);

		virtual void DrawIndexed(VertexArray* vertexArray) = 0;
		static Renderer* Create();

		static Renderer* _spRenderer;

	private:
	};

	class RendererAPI {

	};
}


