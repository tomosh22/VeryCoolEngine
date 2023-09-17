#include "vcepch.h"
#include "Texture.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace VeryCoolEngine {
	Texture2D* VeryCoolEngine::Texture2D::Create(uint32_t width, uint32_t height, TextureFormat format, TextureWrapMode wrapMode)
	{
#ifdef VCE_OPENGL
		return new OpenGLTexture2D(width,height,format,wrapMode);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
	Texture2D* Texture2D::Create(const std::string& path, bool srgb)
	{
#ifdef VCE_OPENGL
		return new OpenGLTexture2D(path,srgb);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}


	TextureCube* TextureCube::Create(const std::string& path, bool srgb)
	{
#ifdef VCE_OPENGL
		return new OpenGLTextureCube(path, srgb);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
}

