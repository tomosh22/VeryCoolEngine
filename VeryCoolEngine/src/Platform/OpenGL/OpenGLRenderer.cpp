#include "vcepch.h"
#include "OpenGLRenderer.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"
#include "VeryCoolEngine/Application.h"
#include <GLFW/glfw3.h>
#include "Platform/Windows/WindowsWindow.h"


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
		if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)return;
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
	}


	void OpenGLRenderer::PlatformInit() {

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);

		//glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
		//glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRenderer::SetClearColor(const glm::vec4 color)
	{
		glClearColor(color.r,color.g,color.b,color.a);
	}

	void OpenGLRenderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::BindViewProjMat(Shader* shader) {
		//glBindBufferBase(GL_UNIFORM_BUFFER, 0, _matrixUBO);
	}
	void OpenGLRenderer::BindLightUBO(Shader* shader) {
		//glBindBufferBase(GL_UNIFORM_BUFFER, 1, _matrixUBO);//#todo light ubo
	}


	void OpenGLRenderer::DrawFullScreenQuad()
	{
		glBindVertexArray(0);
		unsigned int indices[4]{ 0,1,2,3 };
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,indices);
	}

	void OpenGLRenderer::DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology)
	{
		switch (topology) {
			case MeshTopolgy::Triangles:
				glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
				break;
			case MeshTopolgy::TriangleStrips:
				glDrawElements(GL_TRIANGLE_STRIP, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
				break;
		}
		
	}

	void OpenGLRenderer::DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology)
	{
		switch (topology) {
		case MeshTopolgy::Triangles:
			glDrawElementsInstanced(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr, count);
			break;
		case MeshTopolgy::TriangleStrips:
			glDrawElementsInstanced(GL_TRIANGLE_STRIP, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr, count);
			break;
		}

	}

	void OpenGLRenderer::BeginScene(Scene* scene)
	{
		const uint32_t camDataSize = sizeof(glm::mat4) * 3 + sizeof(glm::vec4);//4 bytes of padding
		glm::mat4 viewMat = scene->camera->BuildViewMatrix();
		glm::mat4 projMat = scene->camera->BuildProjectionMatrix();
		glm::mat4 viewProjMat = projMat * viewMat;
		glm::vec3 tempCamPos = scene->camera->GetPosition();
		glm::vec4 camPos = { tempCamPos.x, tempCamPos.y, tempCamPos.z,0 };//4 bytes of padding
		char* camData = new char[camDataSize];
		memcpy(camData + sizeof(glm::mat4) * 0, &viewMat[0][0], sizeof(glm::mat4));
		memcpy(camData + sizeof(glm::mat4) * 1, &projMat[0][0], sizeof(glm::mat4));
		memcpy(camData + sizeof(glm::mat4) * 2, &viewProjMat[0][0], sizeof(glm::mat4));
		memcpy(camData + sizeof(glm::mat4) * 3, &camPos[0], sizeof(glm::vec4));
		_pCameraUBO->UploadData(camData, camDataSize, 1, 0);
		delete[] camData;


		const uint32_t dataSize = (sizeof(unsigned int)*4) + (sizeof(Light) * scene->lights.size());
		char* data = new char[dataSize];
		unsigned int numLightsWithPadding[4] = {scene->numLights,0,0,0 };//12 bytes of padding

		memcpy(data, numLightsWithPadding, sizeof(unsigned int)*4);

		memcpy(data + sizeof(unsigned int) * 4, scene->lights.data(), sizeof(Light) * scene->lights.size());
		_pLightUBO->UploadData(data, dataSize,1,0);
		delete[] data;
	}

	void OpenGLRenderer::EndScene()
	{
	}


	void OpenGLRenderer::OnEvent(Event& e)
	{
		Application* app = Application::GetInstance();
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_ESCAPE) app->_running = false;
		EventDispatcher dispatcher(e);
		if (e.GetType() == EventType::WindowClose) {
			std::function function = [&](WindowCloseEvent& e) -> bool {return app->OnWindowClose(e); };
			dispatcher.Dispatch(function);
		}

		for (auto it = app->_layerStack.end(); it != app->_layerStack.begin();) {
			//#todo can be changed
			(*--it)->OnEvent(e);
			if (e.GetHandled()) break;
		}
	}

	void OpenGLRenderer::InitWindow() {
		Application* app = Application::GetInstance();
		glfwSetInputMode((GLFWwindow*)app->_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		((WindowsWindow*)app->_window)->Init(WindowProperties());


		glfwMakeContextCurrent((GLFWwindow*)app->_window->GetNativeWindow());
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		VCE_CORE_ASSERT(status, "failed to init glad");
		((WindowsWindow*)app->_window)->SetVSync(false);
		}

	void OpenGLRenderer::RenderThreadFunction()
	{
		Application* app = Application::GetInstance();
		


		_spRenderer = Renderer::_spRenderer;
		_spRenderer->PlatformInit();
		_spRenderer->GenericInit();

		

		app->renderInitialised = true;
		while (app->_running) {
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			
			Scene* scene = app->scene;
			while (!scene->ready) { 
				//std::cout << "waiting for main thread" << std::endl;
			}
			
			

			RenderCommand::SetClearColor({ 0.6, 0.2, 0.4, 1 });
			RenderCommand::Clear();
			app->sceneMutex.lock();
			_spRenderer->BeginScene(scene);

			glDisable(GL_DEPTH_TEST);
			SubmitSkybox(scene->skyboxShader, scene->camera, scene->skybox);
			glEnable(GL_DEPTH_TEST);

			SubmitMeshInstanced(scene->_pInstancedMesh, scene->_numInstances);

			unsigned int meshIndex = 0;
			for (size_t i = 0; i < scene->meshes.size(); i++) {
				SubmitMesh(scene->meshes[i]);
			}

			app->_pImGuiLayer->Begin();
			for (Layer* layer : app->_layerStack)
				layer->OnImGuiRender();
			app->_pImGuiLayer->End();

			app->_window->OnUpdate();

			glfwSwapBuffers((GLFWwindow*)app->_window->GetNativeWindow());

			
			app->sceneMutex.unlock();

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		}
	}

	

	
}


