#pragma once
#include "VeryCoolEngine/Renderer/Renderer.h"
#include <glad/glad.h>

namespace VeryCoolEngine {
	class OpenGLRenderer : public Renderer
	{
	public:
		void Init() override;
		void SetClearColor(const glm::vec4 color) override;
		void Clear() override;
		void DrawIndexed(VertexArray* vertexArray) override;
		void BindViewProjMat(Shader* shader) override;
		void DrawFullScreenQuad(Shader* shader, Camera* camera, Texture* debugTex) override;
		void BeginScene(glm::mat4 viewProjMat) override;
		void EndScene() override;
	private:
		GLuint _matrixUBO = -1;
	};
}


