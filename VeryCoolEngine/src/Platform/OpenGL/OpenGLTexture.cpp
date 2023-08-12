#include "vcepch.h"
#include "OpenGLTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VeryCoolEngine/AssetHandling/Assets.h"
#include "Platform/OpenGL/OpenGLShader.h"


namespace VeryCoolEngine {

#pragma region 2d
	VeryCoolEngine::OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat textureFormat, TextureWrapMode wrapMode) : _width(width), _height(height)
	{
		GLenum wrap = wrapMode == TextureWrapMode::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		GLenum format;
		switch (textureFormat) {
		case(TextureFormat::RGBA):
			format = GL_RGBA;
		}
		format = GL_RGBA32F;//#todo stop hardcoding this
		glGenTextures(1, &_id);
		glBindTexture(GL_TEXTURE_2D, _id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);//#todo is unsigned byte always ok?
		//#todo float now
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
	{
		_filePath = path;
		_srgb = srgb;

	}

	void VeryCoolEngine::OpenGLTexture2D::Bind() const
	{
		glBindTexture(GL_TEXTURE_2D, _id);
	}

	void OpenGLTexture2D::BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const
	{
		OpenGLShader* oglShader = dynamic_cast<OpenGLShader*>(shader);
		glUniform1i(glGetUniformLocation(oglShader->GetProgramID(), uniformName.c_str()), bindPoint);
		GLenum texUnit;
		switch (bindPoint) {
			case(0):
				texUnit = GL_TEXTURE0;
				break;
			case(1):
				texUnit = GL_TEXTURE1;
				break;
		}
		glActiveTexture(texUnit);
		glBindTexture(GL_TEXTURE_2D, _id);
	}

	void VeryCoolEngine::OpenGLTexture2D::Unbind() const
	{
	}

	uint32_t VeryCoolEngine::OpenGLTexture2D::GetWidth() const
	{
		return uint32_t();
	}

	uint32_t VeryCoolEngine::OpenGLTexture2D::GetHeight() const
	{
		return uint32_t();
	}
	void OpenGLTexture2D::PlatformInit()
	{
		int width, height, channels;



		_pData = (char*)stbi_load((TEXTUREDIR + _filePath).c_str(), &width, &height, &channels, _srgb ? STBI_rgb : STBI_rgb_alpha);

		//#todo implement other formats
		_format = TextureFormat::RGBA;
		GLenum format = GL_RGBA;
		GLenum internalFormat = GL_RGBA;

		VCE_ASSERT((_pData != nullptr), "couldn't load image");

		_width = width;
		_height = height;

		uint32_t mipLevels = 1;
		while ((width | height) >> mipLevels) mipLevels++;
		glGenTextures(1, &_id);
		glBindTexture(GL_TEXTURE_2D, _id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);


		GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, _pData);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(_pData);
	}
#pragma end region

#pragma region cube
	OpenGLTextureCube::OpenGLTextureCube(const std::string& path, bool srgb) {
		glGenTextures(1, &_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _id);

		std::list<std::string> faces{//#todo stop using jpg, blegh
			"/px.jpg",
			"/nx.jpg",
			"/py.jpg",
			"/ny.jpg",
			"/pz.jpg",
			"/nz.jpg"
		};


		int width, height, channels;
		char* data;
		GLuint faceIndex = 0;//to add to positive x face, couldn't figure out a good variable name
		for (auto& face : faces) {
			std::string file = TEXTUREDIR + path + face;
			data = (char*) stbi_load((TEXTUREDIR+path+face).c_str(), &width, &height, &channels, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, 0, GL_RGB, width, height,0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
			faceIndex++;
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		_width = width;
		_height = height;
		_format = TextureFormat::RGB;
	}

	void OpenGLTextureCube::BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const
	{
		OpenGLShader* oglShader = dynamic_cast<OpenGLShader*>(shader);
		glUniform1i(glGetUniformLocation(oglShader->GetProgramID(), uniformName.c_str()), bindPoint);
		GLenum texUnit;
		switch (bindPoint) {
		case(0):
			texUnit = GL_TEXTURE0;
		}
		glActiveTexture(texUnit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _id);
	}
#pragma end region
}