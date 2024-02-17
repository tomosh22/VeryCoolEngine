#pragma once
namespace VeryCoolEngine {
	class Shader;
	enum class TextureFormat
	{
		RGBA,
		RGB,
		D
	};
	enum class TextureWrapMode
	{
		Clamp,
		Repeat
	};
	class Texture
	{
	public:
		Texture() = default;
		virtual ~Texture() {};
		virtual void Bind() const = 0;
		virtual void BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const = 0;
		virtual void Unbind() const = 0;
		virtual void PlatformInit() = 0;
		
#ifdef VCE_DEBUG
		bool m_bInitialised = false;
#endif
		void* m_pData = nullptr;
		uint32_t m_uDataLength = 0;
		bool m_bIsDepthTexture = false;
		
	private:
	};

	class Texture2D : public Texture {
	public:
		Texture2D() = default;
		Texture2D(std::string filePath, bool srgb) : _filePath(filePath), _srgb(srgb) {}
		virtual ~Texture2D() {};

		static Texture2D* Create(uint32_t width, uint32_t height, TextureFormat textureFormat = TextureFormat::RGBA, TextureWrapMode wrapMode = TextureWrapMode::Clamp);

		static Texture2D* Create(const std::string& path, bool srgb = false);

		static Texture2D* Create();

		void PlatformInit() override {};

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		bool m_bHasFilePath = false;
		std::string _filePath;
		bool _srgb;
	};

	class TextureCube : public Texture {
	public:

		static TextureCube* Create(const std::string& path, bool srgb);

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		std::string _filePath;
		bool _srgb;
	};

}

