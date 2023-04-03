#pragma once

#include "core.h"
#include "Events/Event.h"
#include "Window.h"


namespace VeryCoolEngine {

	class VCE_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
	private:
		Window* _window;
		bool _running = true;
	};
	Application* CreateApplication();

}
