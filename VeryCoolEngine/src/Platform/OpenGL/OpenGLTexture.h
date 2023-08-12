#pragma once
#include <glad/glad.h>
#include "VeryCoolEngine/Renderer/Texture.h"

namespace VeryCoolEngine {
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture() {}
		virtual ~OpenGLTexture() = default;
		GLuint GetID() const { 
			return _id;
		}
	protected:
		GLuint _id;
	};

	class OpenGLTexture2D : public OpenGLTexture, public Texture2D {
	public:
		OpenGLTexture2D() {};
		OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat textureFormat = TextureFormat::RGBA, TextureWrapMode wrapMode = TextureWrapMode::Repeat);

		OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb);

		~OpenGLTexture2D() = default;

		void Bind() const override;
		void BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const override;
		void Unbind() const override;

		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;

		void PlatformInit() override;
	private:
		uint32_t _width;
		uint32_t _height;

		char* _pData = nullptr;
		size_t _dataLength;

		TextureFormat _format;
	};

	class OpenGLTextureCube : public OpenGLTexture, public TextureCube {
	public:
		OpenGLTextureCube(const std::string& path, bool srgb);
		~OpenGLTextureCube() = default;

		void Bind() const override {};
		void BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const override;
		void Unbind() const override {};

		uint32_t GetWidth() const override { return _width; };
		uint32_t GetHeight() const override { return _height; };
		void PlatformInit() override {};
	private:
		uint32_t _width;
		uint32_t _height;
		std::string _filePath;

		char* _pData = nullptr;
		size_t _dataLength;

		TextureFormat _format;
	};
}


