#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include <glad/glad.h>	
#include "Input.h"

#include <glm/glm.hpp>

namespace VeryCoolEngine {


	Application* Application::_spInstance = nullptr;

	

	Application::Application() {
		_spInstance = this;
		_window = Window::Create();
		std::function callback = [this](Event& e) {OnEvent(e); };
		_window->SetEventCallback(callback);

		_pImGuiLayer = new ImGuiLayer();
		PushOverlay(_pImGuiLayer);

		_pVertArray = VertexArray::Create();


		float verts[3 * (3+4)] = {
			-0.5,-0.5,0,    1,0,0,1,
			0.5,-0.5,0,     0,1,0,1,
			0,0.5,0,        0,0,1,1
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

	Application::~Application() { delete _window; }

	void Application::Run() {
		while (_running) {

			glClearColor(0.2, 0.2, 0.4, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			_pVertArray->Bind();
			_pBasicShader->Bind();
			glDrawElements(GL_TRIANGLES, _pVertArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
			
			glDrawArrays(GL_TRIANGLES, 0, 1);
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