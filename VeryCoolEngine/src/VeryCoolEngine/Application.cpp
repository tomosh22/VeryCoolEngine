#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace VeryCoolEngine {
	Application::Application(){
		_window = Window::Create();
	}
	Application::~Application() { delete _window; }
	void Application::Run(){
		while (_running) {
			_window->OnUpdate();
		}
	}
}