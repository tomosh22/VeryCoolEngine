#include "vcepch.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLRenderer.h"
#include "RenderCommand.h"

namespace VeryCoolEngine {
	void Renderer::RenderThreadFunction()
	{
#ifdef VCE_OPENGL
		OpenGLRenderer::OGLRenderThreadFunction();
#endif
	}
	void Renderer::Submit(VertexArray* vertexArray)
	{
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitMesh(Mesh* mesh) {
		mesh->GetShader()->Bind();
		_spRenderer->BindViewProjMat(mesh->GetShader());
		mesh->GetTexture()->BindToShader(mesh->GetShader(),"texture",0);//#todo how to determine bind point
		VertexArray* vertexArray = mesh->GetVertexArray();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitSkybox(Shader* shader, Camera* camera, TextureCube* cubemap)
	{
		shader->Bind();
		cubemap->BindToShader(shader, "cubemap", 0);
		glm::mat4 projMat = camera->BuildProjectionMatrix();
		glm::mat4 viewMat = camera->BuildViewMatrix();

		glm::mat4 inverseProj = glm::inverse(projMat);
		glm::mat4 inverseView = glm::inverse(viewMat);


		shader->UploadMatrix4Uniform(inverseProj, "inverseProjMatrix");
		shader->UploadMatrix4Uniform(inverseView, "inverseViewMatrix");
		_spRenderer->DrawFullScreenQuad();
	}

	Renderer* Renderer::Create() {
#ifdef VCE_OPENGL
		return new OpenGLRenderer();
#endif

	}
}