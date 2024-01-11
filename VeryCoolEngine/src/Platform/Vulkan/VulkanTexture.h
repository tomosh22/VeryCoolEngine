#pragma once
#include "VeryCoolEngine/Renderer/Texture.h"

namespace VeryCoolEngine {
	class VulkanTexture {};
	class VulkanTexture2D : public Texture2D, public VulkanTexture
	{
	public:
		VulkanTexture2D() = default;
		VulkanTexture2D(const std::string& path, bool srgb = false);
		VulkanTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureWrapMode wrapMode);
		~VulkanTexture2D();

		void PlatformInit();

		static vk::Sampler CreateSampler();

		virtual void BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const override;
		virtual void Bind() const override;
		virtual void Unbind() const override;

		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;

		void InitWithFileName();
		void InitWithData();
		void InitWithoutData();

		static VulkanTexture2D* CreateVulkanTexture2D(uint32_t uWidth, uint32_t uHeight, uint32_t uMipCount, vk::Format eFormat, vk::ImageAspectFlags eAspect, vk::ImageUsageFlags eUsage, vk::ImageLayout eLayout, vk::PipelineStageFlags ePipeType);
		static VulkanTexture2D* CreateColourAttachment(uint32_t uWidth, uint32_t uHeight, uint32_t uMipCount, vk::Format eFormat);
		static VulkanTexture2D* CreateDepthAttachment(uint32_t uWidth, uint32_t uHeight);

		vk::Image m_xImage;
		vk::ImageView m_xImageView;
		vk::DeviceMemory m_xDeviceMemory;
		vk::Format m_xFormat;

		vk::Sampler m_xSampler; //do i want this here???

		uint32_t m_uWidth;
		uint32_t m_uHeight;
		uint32_t m_uNumChannels;
	};
}


