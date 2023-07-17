#include "vcepch.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLRenderer.h"
#include "RenderCommand.h"

namespace VeryCoolEngine {
	void Renderer::Submit(VertexArray* vertexArray)
	{
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
	Renderer* Renderer::Create() {
#ifdef VCE_OPENGL
		return new OpenGLRenderer();
#endif

	}
}