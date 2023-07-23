#pragma once
#include <glad/glad.h>
#include "VeryCoolEngine/Renderer/Texture.h"

namespace VeryCoolEngine {
	class OpenGLTexture : public Texture
	{
	public:
		virtual ~OpenGLTexture() = default;
	protected:
		GLuint _id;
	};

	class OpenGLTexture2D : public OpenGLTexture, public Texture2D {
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat textureFormat = TextureFormat::RGBA, TextureWrapMode wrapMode = TextureWrapMode::Clamp);

		OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb);

		~OpenGLTexture2D() = default;

		void Bind() const override;
		void BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const override;
		void Unbind() const override;

		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;
		uint32_t GetDepth() const override;
	private:
		uint32_t _width;
		uint32_t _height;
		std::string _filePath;

		char* _pData = nullptr;
		size_t _dataLength;

		TextureFormat _format;
	};
}


