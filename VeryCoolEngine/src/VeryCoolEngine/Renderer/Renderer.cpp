#include "vcepch.h"
#include "Renderer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "RenderCommand.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/ImGui/ImGuiLayer.h"

namespace VeryCoolEngine {
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
		mesh->m_xTransform.UpdateMatrix();
		mesh->m_xTransform.UpdateRotation();

		mesh->GetShader()->UploadMatrix4Uniform(mesh->m_xTransform._matrix,"_uModelMat");
		_spRenderer->BindViewProjMat(mesh->GetShader());

		//deleted after move to vulkan
		//mesh->GetShader()->UploadIVec2Uniform(mesh->customUniform, "_uAtlasOffset");

		


#ifndef VCE_MATERIAL_TEXTURE_DESC_SET
		mesh->GetTexture()->BindToShader(mesh->GetShader(),"diffuseTex",0);//#todo how to determine bind point
#endif
		//mesh->GetBumpMap()->BindToShader(mesh->GetShader(), "bumpMap", 1);//#todo how to determine bind point
		VertexArray* vertexArray = mesh->GetVertexArray();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::SubmitMeshInstanced(Mesh* mesh, unsigned int count) {
		mesh->GetShader()->Bind();

		_spRenderer->BindViewProjMat(mesh->GetShader());


#ifndef VCE_MATERIAL_TEXTURE_DESC_SET
		mesh->GetTexture()->BindToShader(mesh->GetShader(), "diffuseTex", 0);//#todo how to determine bind point
#endif
		//mesh->GetBumpMap()->BindToShader(mesh->GetShader(), "bumpMap", 1);//#todo how to determine bind point

		//#todo temporary, need to implement proper uniform system
		Application* app = Application::GetInstance();
		mesh->GetShader()->UploadBoolUniform(app->_aoEnabled, "aoEnabled");


		VertexArray* vertexArray = mesh->GetVertexArray();
		vertexArray->Bind();
		RenderCommand::DrawIndexedInstanced(vertexArray,count);
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
#elif defined VCE_VULKAN
	return new VulkanRenderer();
#endif

	}
	void Renderer::GenericInit()
	{
	}

	
}