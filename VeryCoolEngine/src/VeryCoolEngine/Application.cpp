#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace VeryCoolEngine {
	Application::Application(){
		_window = Window::Create();
		std::function callback = [this](Event& e) {OnEvent(e); };
		_window->SetEventCallback(callback);
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		if (e.GetType() == EventType::WindowClose){
			std::function function = [&](WindowCloseEvent& e) -> bool {return Application::OnWindowClose(e); };
			dispatcher.Dispatch(function);
		}
		
		VCE_CORE_INFO(e.GetName());
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		_running = false;
		return true;
	}

	Application::~Application() { delete _window; }
	void Application::Run(){
		while (_running) {
			_window->OnUpdate();
		}
	}
}