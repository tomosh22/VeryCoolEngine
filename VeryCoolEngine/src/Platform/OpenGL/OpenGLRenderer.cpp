#include "vcepch.h"
#include "OpenGLRenderer.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"
#include "VeryCoolEngine/Application.h"
#include <GLFW/glfw3.h>
#include "VeryCoolEngine/ImGui/ImGuiLayer.h"


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

	void OpenGLRenderer::BeginScene(glm::mat4 projViewMat)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, _matrixUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projViewMat[0][0]);
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


	void OpenGLRenderer::OGLRenderThreadFunction()
	{
		Application* app = Application::GetInstance();


		glfwMakeContextCurrent((GLFWwindow*)app->_window->GetNativeWindow());
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		VCE_CORE_ASSERT(status, "failed to init glad");

		_spRenderer = Renderer::_spRenderer;
		_spRenderer->Init();

		app->_pMesh = app->GenerateHeightmap(10, 10);
		app->_pMesh->SetShader(Shader::Create("basic.vert", "basic.frag"));
		app->_pMesh->SetTexture(Texture2D::Create("test1024x1024.png", false));

		app->_pFullscreenShader = Shader::Create("fullscreen.vert", "fullscreen.frag");

		app->_pDebugTexture = Texture2D::Create(app->_window->GetWidth(), app->_window->GetHeight());

		app->_pCubemap = TextureCube::Create("CubemapTest", false);

		app->_pImGuiLayer = new ImGuiLayer();
		app->PushOverlay(app->_pImGuiLayer);

		app->renderInitialised = true;
		app->renderThreadReady = true;
		while (app->renderThreadShouldRun && app->_running) {
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			
			Scene* scene = app->scene;
			while (!scene->ready) {  }
			app->renderThreadReady = false;
			glm::mat4 viewProjMat = scene->camera->BuildProjectionMatrix() * scene->camera->BuildViewMatrix();

			RenderCommand::SetClearColor({ 0.6, 0.2, 0.4, 1 });
			RenderCommand::Clear();

			_spRenderer->BeginScene(viewProjMat);

			SubmitSkybox(scene->skyboxShader, scene->camera, scene->skybox);
			//SubmitSkybox(app->_pFullscreenShader, &app->_Camera, app->_pCubemap);

 			for (Mesh* mesh : scene->meshes) SubmitMesh(mesh);
			//SubmitMesh(app->_pMesh);

			//app->_pImGuiLayer->Begin();
			//for (Layer* layer : app->_layerStack)
				//layer->OnImGuiRender();
			//app->_pImGuiLayer->End();

			glfwSwapBuffers((GLFWwindow*)app->_window->GetNativeWindow());
			app->renderThreadReady = true;

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			//std::cout << "render thread duration: " << std::to_string(duration.count() / 1000.f) << "ms" << std::endl;
		}
	}

	

	
}


