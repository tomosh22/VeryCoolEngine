#include "vcepch.h"
#include "OpenGLRenderer.h"
#include "OpenGLShader.h"

namespace VeryCoolEngine {

	void OpenGLRenderer::Init() {
		glGenBuffers(1, &_matrixUBO);

		//size of view proj matrix
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _matrixUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STATIC_DRAW);//#todo check static draw is right
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, _matrixUBO, 0,sizeof(glm::mat4));

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

	void OpenGLRenderer::DrawFullScreenQuad(Shader* shader, Camera* camera)
	{
		glBindVertexArray(0);
		OpenGLShader* oglShader = dynamic_cast<OpenGLShader*>(shader);
		oglShader->Bind();
		glm::mat4 projMat = camera->BuildProjectionMatrix();
		glm::mat4 viewMat = camera->BuildViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(oglShader->GetProgramID(),"projMatrix"),1,false,&projMat[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(oglShader->GetProgramID(), "viewMatrix"), 1, false, &viewMat[0][0]);
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


