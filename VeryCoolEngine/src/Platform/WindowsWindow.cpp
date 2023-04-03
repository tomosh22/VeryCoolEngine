#include "vcepch.h"
#include "WindowsWindow.h"


namespace VeryCoolEngine {
	static bool glfwInititliazed = false;

	Window* Window::Create(const WindowProperties& p) { return new WindowsWindow(p); }

	WindowsWindow::WindowsWindow(const WindowProperties& p) { Init(p); }
	WindowsWindow::~WindowsWindow() { Shutdown(); }

	void WindowsWindow::Init(const WindowProperties& p) {
		_data._title = p._title;
		_data._width = p._width;
		_data._height = p._height;

		VCE_CORE_INFO("Creating window {0} ({1},{2})", p._title, p._width, p._height);
		if (!glfwInititliazed) {
			VCE_CORE_ASSERT(glfwInit() == GLFW_TRUE, "failed to init glfw");
			glfwInititliazed = true;
		}
		_window = glfwCreateWindow((int)p._width, (int)p._height, p._title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(_window);
		glfwSetWindowUserPointer(_window, &_data);
		SetVSync(true);
	}

	void WindowsWindow::SetVSync(bool enabled) {
		glfwSwapInterval(int(enabled));
		_data._VSync = enabled;
	}

	bool WindowsWindow::GetVSyncEnabled() const { return _data._VSync; }

	void WindowsWindow::Shutdown() { glfwDestroyWindow(_window);}

	void WindowsWindow::OnUpdate() {
		glfwPollEvents();
		glfwSwapBuffers(_window);
	}

}