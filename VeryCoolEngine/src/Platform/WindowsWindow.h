#pragma once

#include "VeryCoolEngine/Window.h"
#include "VeryCoolEngine/Log.h"
#include "VeryCoolEngine/core.h"
#include "GLFW/glfw3.h"

namespace VeryCoolEngine {
	
	class WindowsWindow : public Window {
	public:
		WindowsWindow(const WindowProperties& p);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return _data._width; }
		inline unsigned int GetHeight() const override { return _data._height; }

		inline void SetEventCallback(const EventCallBackFunction& c) override { _data._eventCallback = c; }
		void SetVSync(bool enabled) override;
		bool GetVSyncEnabled() const override;
	private:
		GLFWwindow* _window;
		struct WindowData {
			unsigned int _width, _height;
			std::string _title;
			bool _VSync;
			EventCallBackFunction _eventCallback;
			WindowData() {};
		} _data;
		

		virtual void Init(const WindowProperties& p);
		virtual void Shutdown();
	};


}