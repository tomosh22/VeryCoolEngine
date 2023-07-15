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
			glClearColor(0.1, 0.1, 0.1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
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