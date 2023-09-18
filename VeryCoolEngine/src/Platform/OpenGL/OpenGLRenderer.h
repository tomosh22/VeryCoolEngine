#pragma once
#include "VeryCoolEngine/Renderer/Renderer.h"
#include <glad/glad.h>
#include "OpenGLContext.h"
#include "VeryCoolEngine/Events/Event.h"
#include "Platform/OpenGL/OpenGLManagedUniformBuffer.h"

namespace VeryCoolEngine {
	class OpenGLRenderer : public Renderer
	{
	public:
		void MainLoop() override;
		void InitWindow() override;
		void PlatformInit() override;
		void OnResize(uint32_t uWidth, uint32_t uHeight) override;
		void SetClearColor(const glm::vec4 color) override;
		void Clear() override;
		void DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology = MeshTopolgy::Triangles) override;
		void DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology = MeshTopolgy::Triangles) override;
		void BindViewProjMat(Shader* shader) override;
		void BindLightUBO(Shader* shader) override;
		void DrawFullScreenQuad() override;
		void BeginScene(Scene* scene) override;
		void EndScene() override;
		void RenderThreadFunction() override;

		void OnEvent(Event& e);
	private:
	};
}


