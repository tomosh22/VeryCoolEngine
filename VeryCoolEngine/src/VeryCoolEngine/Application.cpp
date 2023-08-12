#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Input.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include "Platform/OpenGL/OpenGLShader.h"



namespace VeryCoolEngine {

	Renderer* Renderer::_spRenderer = Renderer::Create();
	Renderer* RenderCommand::_spRenderer = Renderer::_spRenderer;

	Application* Application::_spInstance = nullptr;

	

	Application::Application() {
		srand(time(0));
		_spInstance = this;
		_window = Window::Create();
		std::function callback = [this](Event& e) {OnEvent(e); };
		_window->SetEventCallback(callback);
		//_window->SetVSync(true);
		
		//_window->SetVSync(true);
		_pRenderer = Renderer::Create();
		
		_renderThread = std::thread([&]() {
			while (!_renderThreadCanStart) {}
			_pRenderer->InitWindow();
			_pRenderer->RenderThreadFunction();
		});

		//_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 0, 5), 0, 0, 45, 1, 1000, 1280.f/720.f);
		//_Camera = Camera::BuildOrthoCamera(glm::vec3(0, 0, -5), 0, 0, -10, 10, 5, -5, 1, 100);

		


		//_pMesh = Mesh::Create();
		//_pMesh->SetVertexArray(vertexArray);

		//_pMesh->SetShader( Shader::Create("basic.vert", "basic.frag"));

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		scene = new Scene();
		
		bool a = false;
	}

	
	

	void Application::OnEvent(Event& e) {
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_ESCAPE) _running = false;
		EventDispatcher dispatcher(e);
		if (e.GetType() == EventType::WindowClose) {
			std::function function = [&](WindowCloseEvent& e) -> bool {return Application::OnWindowClose(e); };
			dispatcher.Dispatch(function);
		}

		for (auto it = _layerStack.end(); it != _layerStack.begin();) {
			//#todo can be changed
			(*--it)->OnEvent(e);
			if (e.GetHandled()) break;
		}
	}

	void Application::PushLayer(Layer* layer) {
		_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer) {
		_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		_running = false;
		return true;
	}

	Application::~Application() { 
		delete _window;
		delete scene;
		//delete _pCamera;
	}

	

	



	void Application::Run() {
		while (!renderInitialised) {}
		while (_running) {
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			mainThreadReady = true;
			std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _LastFrameTime);
			_DeltaTime = duration.count()/1000.;
			_LastFrameTime = now;
			_Camera.UpdateCamera(_DeltaTime);
			sceneMutex.lock();
			scene->Reset();
			
			scene->camera = &_Camera;
			scene->skyboxShader = _pFullscreenShader;
			scene->skybox = _pCubemap;
			scene->meshes.push_back(_pMesh);
			scene->lights[scene->numLights++] = {
				0,100,0,100,
				1,0,0,1
				};
			scene->lights[scene->numLights++] = {
				100,75,100,100,
				0,1,0,1
			};
			scene->lights[scene->numLights++] = {
				250,100,250,1000,
				0.8,0.8,0.8,1
			};
			//scene->lights[scene->numLights++] = {
			//	1,2,3,4,
			//	1,0,1,1
			//};
			scene->ready = true;
			sceneMutex.unlock();

			
			for (Layer* layer : _layerStack)
				layer->OnUpdate();

			

			

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		}
		_renderThread.join();
	}
}