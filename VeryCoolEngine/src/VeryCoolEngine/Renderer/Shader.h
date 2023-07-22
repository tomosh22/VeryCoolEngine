/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/
#pragma once
#include <string>
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

		virtual void ReloadShader() = 0;

		virtual void Bind() = 0;

		static Shader* Create(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
	protected:

		std::string shaderFiles[(int)ShaderStages::MAXSIZE];
	};
}