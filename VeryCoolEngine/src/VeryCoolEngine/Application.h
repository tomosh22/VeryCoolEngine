#pragma once

#include "core.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Window.h"


namespace VeryCoolEngine {

	class VCE_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);
	private:
		Window* _window;
		bool _running = true;
		bool OnWindowClose(WindowCloseEvent& e);
	};
	Application* CreateApplication();

}
