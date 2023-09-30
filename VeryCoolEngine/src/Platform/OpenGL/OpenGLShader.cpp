/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/

#include "vcepch.h"
#include "OpenGLShader.h"


#include "VeryCoolEngine/AssetHandling/Assets.h"

namespace VeryCoolEngine {



	GLuint shaderTypes[(int)ShaderStages::MAXSIZE] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
		GL_GEOMETRY_SHADER,
		GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER
	};

	std::string ShaderNames[(int)ShaderStages::MAXSIZE] = {
		"Vertex",
		"Fragment",
		"Geometry",
		"Tess. Control",
		"Tess. Eval"
	};

	OpenGLShader::OpenGLShader(const std::string& vertex, const std::string& fragment, const std::string& geometry, const std::string& domain, const std::string& hull) :
		Shader(vertex, fragment, geometry, domain, hull) {

		for (int i = 0; i < (int)ShaderStages::MAXSIZE; ++i) {
			shaderIDs[i] = 0;
			shaderValid[i] = 0;
		}
		programID = 0;

		//ReloadShader();
	}

	OpenGLShader::~OpenGLShader() {
		DeleteIDs();
	}

	void OpenGLShader::Bind() {
		glUseProgram(programID);
	}
	void OpenGLShader::UploadMatrix4Uniform(const glm::mat4& matrix, const std::string& name) const	{
		glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, false, &matrix[0][0]);
	}

	void OpenGLShader::UploadVec3Uniform(const glm::vec3& matrix, const std::string& name) const{
		glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &matrix[0]);
	}

	void OpenGLShader::UploadIVec2Uniform(const glm::ivec2& ivec2, const std::string& name) const
	{
		glUniform2iv(glGetUniformLocation(programID, name.c_str()), 1, &ivec2[0]);
	}

	void OpenGLShader::UploadBoolUniform(const bool b, const std::string& name) const
	{
		glUniform1i(glGetUniformLocation(programID, name.c_str()), b ? 1 : 0);
	}

	void OpenGLShader::ReloadShader() {
		DeleteIDs();
		programID = glCreateProgram();
		std::string fileContents = "";
		for (int i = 0; i < (int)ShaderStages::MAXSIZE; ++i) {
			if (!shaderFiles[i].empty()) {
				if (ReadTextFile(SHADERDIR + shaderFiles[i], fileContents)) {
					shaderIDs[i] = glCreateShader(shaderTypes[i]);

					std::cout << "Reading " << ShaderNames[i] << " shader " << shaderFiles[i] << std::endl;

					const char* stringData = fileContents.c_str();
					int			stringLength = (int)fileContents.length();
					glShaderSource(shaderIDs[i], 1, &stringData, &stringLength);
					glCompileShader(shaderIDs[i]);

					glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &shaderValid[i]);

					if (shaderValid[i] != GL_TRUE) {
						std::cout << ShaderNames[i] << " shader " << " has failed!" << std::endl;
					}
					else {
						glAttachShader(programID, shaderIDs[i]);
					}
					PrintCompileLog(shaderIDs[i]);
				}
			}
		}
		glLinkProgram(programID);
		glGetProgramiv(programID, GL_LINK_STATUS, &programValid);

		PrintLinkLog(programID);

		if (programValid != GL_TRUE) {
			std::cout << "This shader has failed!" << std::endl;
		}
		else {
			std::cout << "Shader loaded!" << std::endl;
		}
	}

	void OpenGLShader::PlatformInit()
	{
	}

	void	OpenGLShader::DeleteIDs() {
		if (!programID) {
			return;
		}
		for (int i = 0; i < (int)ShaderStages::MAXSIZE; ++i) {
			if (shaderIDs[i]) {
				glDetachShader(programID, shaderIDs[i]);
				glDeleteShader(shaderIDs[i]);
			}
		}
		glDeleteProgram(programID);
		programID = 0;
	}

	void	OpenGLShader::PrintCompileLog(GLuint object) {
		int logLength = 0;
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength) {
			char* tempData = new char[logLength];
			glGetShaderInfoLog(object, logLength, NULL, tempData);
			std::cout << "Compile Log:\n" << tempData << std::endl;
			delete tempData;
		}
	}

	void	OpenGLShader::PrintLinkLog(GLuint program) {
		int logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength) {
			char* tempData = new char[logLength];
			glGetProgramInfoLog(program, logLength, NULL, tempData);
			std::cout << "Link Log:\n" << tempData << std::endl;
			delete tempData;
		}
	}
}