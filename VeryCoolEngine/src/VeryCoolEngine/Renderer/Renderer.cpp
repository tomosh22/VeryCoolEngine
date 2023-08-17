#include "vcepch.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLRenderer.h"
#include "RenderCommand.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/ImGui/ImGuiLayer.h"

namespace VeryCoolEngine {
	uint32_t Renderer::_sMAXLIGHTS = 100;
	//void Renderer::RenderThreadFunction()
	//{
#ifdef VCE_OPENGL
		//OpenGLRenderer::OGLRenderThreadFunction();
#endif
	//}
	void Renderer::Submit(VertexArray* vertexArray)
	{
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitMesh(Mesh* mesh) {
		mesh->GetShader()->Bind();

		//#todo move these 2 somewhere else
		mesh->transform.UpdateMatrix();
		mesh->transform.UpdateRotation();

		mesh->GetShader()->UploadMatrix4Uniform(mesh->transform._matrix,"_uModelMat");
		_spRenderer->BindViewProjMat(mesh->GetShader());

		mesh->GetShader()->UploadIVec2Uniform(mesh->customUniform, "_uAtlasOffset");


		mesh->GetTexture()->BindToShader(mesh->GetShader(),"diffuseTex",0);//#todo how to determine bind point
		//mesh->GetBumpMap()->BindToShader(mesh->GetShader(), "bumpMap", 1);//#todo how to determine bind point
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
	void Renderer::GenericInit()
	{
		Application* app = Application::GetInstance();

		app->_pImGuiLayer = new ImGuiLayer();
		app->PushOverlay(app->_pImGuiLayer);

		//app->_pMesh = Mesh::GenerateHeightmap(100, 100);
		for (Shader* shader : app->_shaders) shader->ReloadShader();

		for (Mesh* mesh : app->_meshes) {
			mesh->PlatformInit();
			mesh->GetShader()->ReloadShader();

			mesh->GetTexture()->PlatformInit();
			//mesh->GetBumpMap()->PlatformInit();
		}

		
		app->_pFullscreenShader->ReloadShader();

		app->_pDebugTexture = Texture2D::Create(app->_window->GetWidth(), app->_window->GetHeight());

		app->_pCubemap->PlatformInit();

		_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), 1,0);//#todo frames in flight
		_pLightUBO = ManagedUniformBuffer::Create(sizeof(Light) * _sMAXLIGHTS,1,1);//#todo frames in flight
	}
}