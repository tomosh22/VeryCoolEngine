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
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
	}


	void OpenGLRenderer::PlatformInit() {
		glGenBuffers(1, &_matrixUBO);

		//size of view proj matrix
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _matrixUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STATIC_DRAW);//#todo check static draw is right
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, _matrixUBO, 0,sizeof(glm::mat4));

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
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _matrixUBO);
	}
	void OpenGLRenderer::BindLightUBO(Shader* shader) {
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, _matrixUBO);//#todo light ubo
	}


	void OpenGLRenderer::DrawFullScreenQuad()
	{
		glBindVertexArray(0);
		unsigned int indices[4]{ 0,1,2,3 };
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,indices);
	}

	void OpenGLRenderer::DrawIndexed(VertexArray* vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::BeginScene(Scene* scene)
	{
		glm::mat4 viewProjMat = scene->camera->BuildProjectionMatrix() * scene->camera->BuildViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, _matrixUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &viewProjMat[0][0]);

		//VCE_ASSERT((scene->lights[0].color.r == 0.1), "waddafack");

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
		app->renderThreadReady = true;
		while (app->_running) {
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			
			Scene* scene = app->scene;
			while (!scene->ready) { 
				//std::cout << "waiting for main thread" << std::endl;
			}
			
			app->renderThreadReady = false;
			

			RenderCommand::SetClearColor({ 0.6, 0.2, 0.4, 1 });
			RenderCommand::Clear();
			app->sceneMutex.lock();
			_spRenderer->BeginScene(scene);

			glDisable(GL_DEPTH_TEST);
			SubmitSkybox(scene->skyboxShader, scene->camera, scene->skybox);
			glEnable(GL_DEPTH_TEST);
			//SubmitSkybox(app->_pFullscreenShader, &app->_Camera, app->_pCubemap);

			GLsync* fences = new GLsync[scene->meshes.size()];
			unsigned int meshIndex = 0;
			for (size_t i = 0; i < scene->meshes.size(); i++) {
				SubmitMesh(scene->meshes[i]);
				fences[meshIndex++] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
			}
			//SubmitMesh(app->_pMesh);

			app->_pImGuiLayer->Begin();
			for (Layer* layer : app->_layerStack)
				layer->OnImGuiRender();
			app->_pImGuiLayer->End();

			app->_window->OnUpdate();

			glfwSwapBuffers((GLFWwindow*)app->_window->GetNativeWindow());

			for (size_t i = 0; i < scene->meshes.size(); i++)
			{
				glWaitSync(fences[i],0, GL_TIMEOUT_IGNORED);
			}
			app->sceneMutex.unlock();
			app->renderThreadReady = true;

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			//std::cout << "render thread duration: " << std::to_string(duration.count() / 1000.f) << "ms" << std::endl;
		}
	}

	

	
}


