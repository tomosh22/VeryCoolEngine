/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/
#include "vcepch.h"
#include "Shader.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace VeryCoolEngine {
	Shader::Shader(const std::string& vertex, const std::string& fragment, const std::string& geometry, const std::string& domain, const std::string& hull)
	{
		shaderFiles[(int)ShaderStages::Vertex] = vertex;
		shaderFiles[(int)ShaderStages::Fragment] = fragment;
		shaderFiles[(int)ShaderStages::Geometry] = geometry;
		shaderFiles[(int)ShaderStages::Domain] = domain;
		shaderFiles[(int)ShaderStages::Hull] = hull;
	}

	Shader::~Shader()
	{
	}

	Shader* Shader::Create(const std::string& vertex, const std::string& fragment, const std::string& geometry, const std::string& domain, const std::string& hull) {
#ifdef VCE_OPENGL
		return new OpenGLShader(vertex, fragment, geometry, domain,  hull);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
}

