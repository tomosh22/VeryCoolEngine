#pragma once

#include "VeryCoolEngine/Window.h"
#include "VeryCoolEngine/Log.h"
#include "VeryCoolEngine/core.h"
#ifdef VCE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include "GLFW/glfw3.h"
#ifdef VCE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "VeryCoolEngine/Renderer/GraphicsContext.h"

namespace VeryCoolEngine {
	
	class WindowsWindow : public Window {
	public:
		WindowsWindow(const WindowProperties& p);
		virtual ~WindowsWindow();

		void* GetNativeWindow() const override { return _pWindow; }

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return _data._width; }
		inline unsigned int GetHeight() const override { return _data._height; }

		inline void SetWidth(unsigned int width)  override { _data._width = width; }
		inline void SetHeight(unsigned int height)  override { _data._height = height; }

		inline void SetEventCallback(const EventCallBackFunction& c) override {
			_data._eventCallback = c;
		}
		void SetVSync(bool enabled) override;
		bool GetVSyncEnabled() const override;
		virtual void Init(const WindowProperties& p);
	private:
		GLFWwindow* _pWindow = nullptr;
		struct WindowData {
			unsigned int _width, _height;
			std::string _title;
			bool _VSync;
			EventCallBackFunction _eventCallback;
		} _data;
		

		
		virtual void Shutdown();
	};


}