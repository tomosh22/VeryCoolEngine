#pragma once
#include "VeryCoolEngine/Renderer/Renderer.h"
#include <glad/glad.h>

namespace VeryCoolEngine {
	class OpenGLRenderer : public Renderer
	{
	public:
		void SetClearColor(const glm::vec4 color) override;
		void Clear() override;
		void DrawIndexed(VertexArray* vertexArray) override;
		void BeginScene() override;
		void EndScene() override;
	};
}


