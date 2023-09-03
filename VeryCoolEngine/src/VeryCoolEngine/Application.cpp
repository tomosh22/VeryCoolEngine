#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Input.h"
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
		_window->SetVSync(true);
		
		//_window->SetVSync(true);
		_pRenderer = Renderer::Create();
		
		_renderThread = std::thread([&]() {
			while (true) {
				printf("implement mutex\n");
				if (_renderThreadCanStart)break;//#todo implement mutex here
			}
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
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_Q) _mouseEnabled = !_mouseEnabled;

		
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
		while (true) { 
			printf("Waiting on render thread init\n");
			if (renderInitialised)break;//#todo implement mutex here
		}
		while (_running) {

			

			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			mainThreadReady = true;
			std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _LastFrameTime);
			_DeltaTime = duration.count()/1000.;
			_LastFrameTime = now;


			_Camera.UpdateCamera(_DeltaTime);
			GameLoop();
			sceneMutex.lock();
			scene->Reset();
			
			scene->camera = &_Camera;
			scene->skyboxShader = _pFullscreenShader;
			scene->skybox = _pCubemap;

			scene->_pInstancedMesh = _pMesh;
			scene->_numInstances = _numInstances;


			//scene->meshes.push_back(_pMesh);
			for (Mesh* mesh : _meshes) { 
				scene->meshes.push_back(mesh);
			}
			for (Mesh* mesh : _meshes) {
				scene->meshes.push_back(mesh);
			}

			for (Renderer::Light& light : _lights) {
				scene->lights[scene->numLights++] = light;
			}
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