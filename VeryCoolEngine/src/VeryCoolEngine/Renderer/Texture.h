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
	enum class TextureStreamPriority : uint8_t {
		NotStreamed = 0,
		Low = 1,
		Medium = 2,
		High = 3
	};
	class Texture2D;
	struct StreamingInfo {
		TextureStreamPriority m_ePrio = TextureStreamPriority::NotStreamed;
		bool m_bIsStreamingTex = false;
		void* m_pNewData = nullptr;
		bool m_bStreamDone = false;
		Texture2D* m_pxNewTex = nullptr;
		int m_uNewWidth = -1;
		int m_uNewHeight = -1;
		
	};
	class Texture
	{
	public:
		Texture() = default;
		virtual ~Texture() {};
		virtual void Bind() const = 0;
		virtual void BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const = 0;
		virtual void Unbind() const = 0;
		virtual void PlatformInit(bool bAsyncLoader = false) = 0;
		virtual void ReceiveStream() = 0;
		
		bool m_bInitialised = false;
		void* m_pData = nullptr;
		uint32_t m_uDataLength = 0;
		bool m_bIsDepthTexture = false;
		uint32_t m_uNumMips = 0;

		
		StreamingInfo m_xStreamInfo;
		class Material* m_pxParentMaterial = nullptr;
		
	private:
	};

	class Texture2D : public Texture {
	public:
		Texture2D() = default;
		Texture2D(std::string filePath, bool srgb) : _filePath(filePath), _srgb(srgb) {}
		virtual ~Texture2D() {};

		static Texture2D* Create(uint32_t width, uint32_t height, TextureFormat textureFormat = TextureFormat::RGBA, TextureWrapMode wrapMode = TextureWrapMode::Clamp);

		static Texture2D* Create(const std::string& path, TextureStreamPriority eStreamPrio, bool srgb = false);

		static Texture2D* Create();

		void PlatformInit(bool bAsyncLoader = false) override {};

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

