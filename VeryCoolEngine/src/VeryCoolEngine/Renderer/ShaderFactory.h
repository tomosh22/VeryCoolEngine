#pragma once
#include "Platform/OpenGL/OpenGLShader.h"
namespace VeryCoolEngine {
	class ShaderFactory {
	public:
		static Shader* CreateShader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "") {

#ifdef VCE_OPENGL
			return new OpenGLShader(vertex, fragment, geometry, domain, hull);
#endif
		}

	};
}