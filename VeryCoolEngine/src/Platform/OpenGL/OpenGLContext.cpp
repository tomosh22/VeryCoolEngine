#include "vcePCH.h"
#include "OpenGLContext.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>


VeryCoolEngine::OpenGLContext::OpenGLContext(GLFWwindow* window) : _pWindow(window){
}

void VeryCoolEngine::OpenGLContext::Init(){
	glfwMakeContextCurrent(_pWindow);
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	VCE_CORE_ASSERT(status, "failed to init glad");
}

void VeryCoolEngine::OpenGLContext::SwapBuffers(){
	
	glfwSwapBuffers(_pWindow);
}
