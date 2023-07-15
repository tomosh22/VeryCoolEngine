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

		glGenVertexArrays(1, &_vertArray);
		glBindVertexArray(_vertArray);

		glGenBuffers(1, &_vertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, _vertBuffer);

		float verts[9] = {
			-0.5,-0.5,0,
			0.5,-0.5,0,
			0,0.5,0
		};
		glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), verts, GL_STREAM_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		glGenBuffers(1, &_indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		unsigned int indices[3] = { 0,1,2 };
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * 3, indices, GL_STREAM_DRAW);

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

			glBindVertexArray(_vertArray);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

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