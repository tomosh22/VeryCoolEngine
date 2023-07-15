#pragma once

#include "VeryCoolEngine/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace VeryCoolEngine {
	
	class OpenGLContext : public GraphicsContext {

	public:
		OpenGLContext(GLFWwindow* window);
			
		void Init() override;
		void SwapBuffers() override;
	private:
		GLFWwindow* _pWindow;
	};
}