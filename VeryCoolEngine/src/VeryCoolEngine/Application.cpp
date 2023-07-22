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
		_spInstance = this;
		_window = Window::Create();
		std::function callback = [this](Event& e) {OnEvent(e); };
		_window->SetEventCallback(callback);
		_window->SetVSync(true);

		_pRenderer = Renderer::_spRenderer;


		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 0, 5), 0, 0, 45, 0, 1000);
		//_Camera = Camera::BuildOrthoCamera(glm::vec3(0, 0, -5), 0, 0, -10, 10, 5, -5, 1, 100);

		_pImGuiLayer = new ImGuiLayer();
		PushOverlay(_pImGuiLayer);

		_pVertArray = VertexArray::Create();


		float verts[3 * (3+4)] = {
			-5,-5,0,    1,0,0,1,
			5,-5,0,     0,1,0,1,
			0,5,0,        0,0,1,1
		};
		
		_pVertBuffer = VertexBuffer::Create(verts,sizeof(verts));

		BufferLayout layout = {
			{ShaderDataType::Float3, "_aPosition"},
			{ShaderDataType::Float4, "_aColor"},
		};
		_pVertBuffer->SetLayout(layout);
		
		_pVertArray->AddVertexBuffer(_pVertBuffer);
		
		unsigned int indices[3] = { 0,1,2 };
		_pIndexBuffer = IndexBuffer::Create(indices, 3);
		_pVertArray->SetIndexBuffer(_pIndexBuffer);

		_pVertArray->Unbind();

		_pBasicShader = Shader::Create("basic.vert", "basic.frag");
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
		//delete _pCamera;
	}

	void Application::Run() {
		while (_running) {

			std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _LastFrameTime);
			_DeltaTime = duration.count()/1000.;
			//std::cout << "delta time: " << _DeltaTime << std::endl;
			_LastFrameTime = now;

			RenderCommand::SetClearColor({ 0.6, 0.2, 0.4, 1 });
			RenderCommand::Clear();
			
			_Camera.UpdateCamera(_DeltaTime);

			_pVertArray->Bind();
			_pBasicShader->Bind();
			GLuint viewMatLoc = glGetUniformLocation(((OpenGLShader*)_pBasicShader)->GetProgramID(), "viewMat");
			GLuint projMatLoc = glGetUniformLocation(((OpenGLShader*)_pBasicShader)->GetProgramID(), "projMat");
			glm::mat4 viewMat = _Camera.BuildViewMatrix();
			glm::mat4 projMat = _Camera.BuildProjectionMatrix();
			glUniformMatrix4fv(viewMatLoc, 1, false, (float*)(&viewMat[0]));
			glUniformMatrix4fv(projMatLoc, 1, false, (float*)(&projMat[0]));
			Renderer::Submit(_pVertArray);
			
			for (Layer* layer : _layerStack)
				layer->OnUpdate();

			_pImGuiLayer->Begin();
			for (Layer* layer : _layerStack)
				layer->OnImGuiRender();
			_pImGuiLayer->End();

			_window->OnUpdate();
		}
	}
}