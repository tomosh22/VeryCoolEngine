/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/
#pragma once
#include "VeryCoolEngine/Renderer/Shader.h"
#include <glad/glad.h>

namespace VeryCoolEngine {
		class OpenGLShader : public Shader
		{
		public:
			friend class OGLRenderer;
			OpenGLShader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
			~OpenGLShader();

			void ReloadShader() override;

			bool LoadSuccess() const {
				return programValid == GL_TRUE;
			}

			int GetProgramID() const {
				return programID;
			}

			static void	PrintCompileLog(GLuint object);
			static void	PrintLinkLog(GLuint program);

			void Bind() override;

		protected:
			void	DeleteIDs();

			GLuint	programID;
			GLuint	shaderIDs[(int)ShaderStages::MAXSIZE];
			int		shaderValid[(int)ShaderStages::MAXSIZE];
			int		programValid;
		};
	
}