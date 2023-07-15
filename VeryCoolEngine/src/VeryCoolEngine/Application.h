#pragma once

#include "core.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Window.h"
#include "VeryCoolEngine/Layers/LayerStack.h"
#include "VeryCoolEngine/Events/Event.h"
#include "VeryCoolEngine/Events/ApplicationEvent.h"


namespace VeryCoolEngine {

	class VCE_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		Window& GetWindow() const { return *_window; };
		static Application* GetInstance() { return _spInstance; }
	private:
		Window* _window;
		bool _running = true;
		bool OnWindowClose(WindowCloseEvent& e);
		LayerStack _layerStack;
		static Application* _spInstance;
	};
	Application* CreateApplication();

}
