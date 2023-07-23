#include "vcepch.h"
#include "OpenGLRenderer.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"

namespace VeryCoolEngine {

	void GLAPIENTRY
		MessageCallback(GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam)
	{
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
	}


	void OpenGLRenderer::Init() {
		glGenBuffers(1, &_matrixUBO);

		//size of view proj matrix
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _matrixUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STATIC_DRAW);//#todo check static draw is right
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, _matrixUBO, 0,sizeof(glm::mat4));

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);

		//glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
		//glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
	}

	void OpenGLRenderer::SetClearColor(const glm::vec4 color)
	{
		glClearColor(color.r,color.g,color.b,color.a);
	}

	void OpenGLRenderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void OpenGLRenderer::BindViewProjMat(Shader* shader) {
		glBindBufferBase(GL_UNIFORM_BUFFER, 5, _matrixUBO);
	}

	void OpenGLRenderer::DrawFullScreenQuad(Shader* shader, Camera* camera, TextureCube* cubemap)
	{
		glBindVertexArray(0);
		OpenGLShader* oglShader = dynamic_cast<OpenGLShader*>(shader);
		oglShader->Bind();
		OpenGLTextureCube* oglTex = dynamic_cast<OpenGLTextureCube*>(cubemap);
		oglTex->BindToShader(shader,"cubemap",0);



		glm::mat4 projMat = camera->BuildProjectionMatrix();
		glm::mat4 viewMat = camera->BuildViewMatrix();

		glm::mat4 inverseProj = glm::inverse(projMat);
		glm::mat4 inverseView = glm::inverse(viewMat);


		glm::vec3 camPos = camera->GetPosition();


		glUniformMatrix4fv(glGetUniformLocation(oglShader->GetProgramID(),"inverseProjMatrix"),1,false,&inverseProj[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(oglShader->GetProgramID(), "inverseViewMatrix"), 1, false, &inverseView[0][0]);
		glUniform3fv(glGetUniformLocation(oglShader->GetProgramID(), "cameraPos"),1, &camPos[0]);
		unsigned int indices[4]{ 0,1,2,3 };
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,indices);
	}

	void OpenGLRenderer::DrawIndexed(VertexArray* vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::BeginScene(glm::mat4 projViewMat)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, _matrixUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projViewMat[0][0]);
	}

	void OpenGLRenderer::EndScene()
	{
	}

	
}


