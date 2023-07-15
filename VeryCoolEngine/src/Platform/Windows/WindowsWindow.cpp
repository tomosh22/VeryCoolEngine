#include "vcepch.h"
#include "WindowsWindow.h"
#include "VeryCoolEngine/Events/ApplicationEvent.h"
#include "VeryCoolEngine/Events/MouseEvent.h"
#include "VeryCoolEngine/Events/KeyEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"


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
			glfwSetErrorCallback([](int error, const char* desc) {VCE_CORE_ERROR("glfw error {0} {1}",error,desc); });
			glfwInititliazed = true;
		}
		_pWindow = glfwCreateWindow((int)p._width, (int)p._height, p._title.c_str(), nullptr, nullptr);
		_pContext = new OpenGLContext(_pWindow);
		_pContext->Init();

		
		glfwSetWindowUserPointer(_pWindow, &_data);
		SetVSync(true);

		glfwSetWindowSizeCallback(_pWindow, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data._width = width;
			data._height = height;
			data._eventCallback(WindowResizeEvent(width, height));
			
		});

		glfwSetWindowCloseCallback(_pWindow, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data._eventCallback(WindowCloseEvent());
		});

		glfwSetKeyCallback(_pWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
			case GLFW_PRESS:
				data._eventCallback(KeyPressedEvent(key,0));
				break;
			case GLFW_REPEAT:
				data._eventCallback(KeyPressedEvent(key, 1));
				break;
			case GLFW_RELEASE:
				data._eventCallback(KeyReleasedEvent(key));
				break;
			}
		});

		glfwSetMouseButtonCallback(_pWindow, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
			case GLFW_PRESS:
				data._eventCallback(MouseButtonPressedEvent(button));
				break;
			case GLFW_RELEASE:
				data._eventCallback(MouseButtonReleasedEvent(button));
				break;
			}
		});

		glfwSetScrollCallback(_pWindow, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data._eventCallback(MouseScrolledEvent((float)xOffset, (float)yOffset));
		});

		glfwSetCursorPosCallback(_pWindow, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data._eventCallback(MouseMovedEvent((float)xPos, (float)yPos));
		});


	}

	void WindowsWindow::SetVSync(bool enabled) {
		glfwSwapInterval(int(enabled));
		_data._VSync = enabled;
	}

	bool WindowsWindow::GetVSyncEnabled() const { return _data._VSync; }

	void WindowsWindow::Shutdown() { glfwDestroyWindow(_pWindow);}

	void WindowsWindow::OnUpdate() {
		glfwPollEvents();
		_pContext->SwapBuffers();
		
	}

}