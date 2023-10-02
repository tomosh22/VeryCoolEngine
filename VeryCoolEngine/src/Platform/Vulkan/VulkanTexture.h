#pragma once
#include "VeryCoolEngine/Renderer/Texture.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace VeryCoolEngine {
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D() = default;
		VulkanTexture2D(const std::string& path, bool srgb = false);
		~VulkanTexture2D();

		void PlatformInit();

		static vk::Sampler CreateSampler();

		virtual void BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const override;
		virtual void Bind() const override;
		virtual void Unbind() const override;

		 uint32_t GetWidth() const override;
		 uint32_t GetHeight() const override;

		vk::Image m_xImage;
		vk::ImageView m_xImageView;
		vk::DeviceMemory m_xDeviceMemory;
		vk::Format m_xFormat;

		vk::Sampler m_xSampler; //do i want this here???

		uint32_t m_uWidth;
		uint32_t m_uHeight;
		uint32_t m_uNumChannels;

		char* m_pData = nullptr;
		size_t m_uDataLength;
	};
}


