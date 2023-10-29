/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/
#pragma once
#include <string>
#include <glm/mat4x4.hpp>
namespace VeryCoolEngine {
	enum class ShaderStages {
		Vertex,
		Fragment,
		Geometry,
		Domain,
		Hull,
		Mesh,
		Task,
		MAXSIZE,
		//Aliases
		TessControl = Domain,
		TessEval = Hull,
	};

	class Shader {
	public:
		Shader() {
		}
		Shader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
		virtual ~Shader();

		virtual void PlatformInit() = 0;

		virtual void ReloadShader() = 0;

		virtual void Bind() = 0;

		virtual void UploadMatrix4Uniform(const glm::mat4& matrix, const std::string& name) const = 0;
		virtual void UploadVec3Uniform(const glm::vec3& matrix, const std::string& name) const = 0;
		virtual void UploadIVec2Uniform(const glm::ivec2& ivec2, const std::string& name) const = 0;
		virtual void UploadBoolUniform(const bool b, const std::string& name) const = 0;

		static Shader* Create(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");

		bool m_bTesselation = false;
	protected:

		std::string shaderFiles[(int)ShaderStages::MAXSIZE];
	};
}