#include "vcepch.h"
#include "WindowsInput.h"
#include <GLFW/glfw3.h>
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {

	Input* Input::_sInstance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(int keyCode){
		GLFWwindow* window = reinterpret_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
		int state = glfwGetKey(window, keyCode);
		return state == GLFW_PRESS;
	}
	bool WindowsInput::IsKeyReleasedImpl(int keyCode) {
		GLFWwindow* window = reinterpret_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
		int state = glfwGetKey(window, keyCode);
		return state == GLFW_RELEASE;
	}
	bool WindowsInput::IsMouseButtonPressedImpl(int button) {
		GLFWwindow* window = reinterpret_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}
	std::pair<double, double> WindowsInput::GetMousePosImpl() {
		GLFWwindow* window = reinterpret_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return { x,y };
	}
}
