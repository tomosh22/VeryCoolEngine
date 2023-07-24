#pragma once
#include "VeryCoolEngine/Renderer/Renderer.h"
#include <glad/glad.h>
#include "OpenGLContext.h"
#include "VeryCoolEngine/Events/Event.h"

namespace VeryCoolEngine {
	class OpenGLRenderer : public Renderer
	{
	public:
		void Init() override;
		void SetClearColor(const glm::vec4 color) override;
		void Clear() override;
		void DrawIndexed(VertexArray* vertexArray) override;
		void BindViewProjMat(Shader* shader) override;
		void DrawFullScreenQuad() override;
		void BeginScene(glm::mat4 viewProjMat) override;
		void EndScene() override;
		static void OGLRenderThreadFunction();

		void OnEvent(Event& e);
	private:
		GLuint _matrixUBO = -1;
	};
}


