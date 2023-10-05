#include "vcepch.h"
#include "VulkanTexture.h"
#ifdef VCE_VULKAN
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>
#include "VeryCoolEngine/AssetHandling/Assets.h"
#include "VulkanBuffer.h"

namespace VeryCoolEngine {

	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureWrapMode wrapMode) {
	
		m_uWidth = width;
		m_uHeight = height;
		if(format == TextureFormat::D)
		m_xFormat = vk::Format::eD32Sfloat;
	}

	VulkanTexture2D::VulkanTexture2D(const std::string& path, bool srgb) {
		m_bHasFilePath = true;
		_filePath = path;
		_srgb = srgb;
	}

	VulkanTexture2D::~VulkanTexture2D() {
		stbi_image_free(m_pData);
		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		xDevice.destroySampler(m_xSampler);
		xDevice.destroyImageView(m_xImageView);
		xDevice.destroyImage(m_xImage);
		xDevice.freeMemory(m_xDeviceMemory);
	}

	void VulkanTexture2D::InitWithData() {
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device xDevice = pxRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pxRenderer->GetPhysicalDevice();

		vk::Format xFormat = vk::Format::eR8G8B8A8Srgb;//#todo handle different formats

		int uWidth, uHeight, uNumChannels;

		m_pData = (char*)stbi_load((TEXTUREDIR + _filePath).c_str(), &uWidth, &uHeight, &uNumChannels, STBI_rgb_alpha);

		m_uWidth = uWidth; m_uHeight = uHeight;

		vk::DeviceSize uSize = uWidth * uHeight * 4;

		VulkanBuffer* pxStagingBuffer = new VulkanBuffer(uSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		pxStagingBuffer->UploadData(m_pData, uSize);

		vk::ImageCreateInfo xImageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(xFormat)
			.setTiling(vk::ImageTiling::eOptimal)
			.setExtent({ m_uWidth,m_uHeight,1 })
			.setMipLevels(1)
			.setArrayLayers(1)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSamples(vk::SampleCountFlagBits::e1);

		m_xImage = xDevice.createImage(xImageInfo);

		vk::MemoryRequirements xMemRequirements = xDevice.getImageMemoryRequirements(m_xImage);

		uint32_t memoryType = -1;
		for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
			if ((xMemRequirements.memoryTypeBits & (1 << i)) && (xPhysDevice.getMemoryProperties().memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal) {
				memoryType = i;
				break;
			}
		}
		VCE_ASSERT(memoryType != -1, "couldn't find physical memory type");

		vk::MemoryAllocateInfo xAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(xMemRequirements.size)
			.setMemoryTypeIndex(memoryType);

		m_xDeviceMemory = xDevice.allocateMemory(xAllocInfo);

		xDevice.bindImageMemory(m_xImage, m_xDeviceMemory, 0);

		vk::CommandBuffer xCmd = pxRenderer->BeginSingleUseCmdBuffer();

		pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer);
		VulkanBuffer::CopyBufferToImage(pxStagingBuffer, this, m_uWidth, m_uHeight);
		pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader);//todo will probably need to change this to vertex shader in the future

		delete pxStagingBuffer;


		vk::ImageSubresourceRange xSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		vk::ImageViewCreateInfo xViewCreate = vk::ImageViewCreateInfo()
			.setImage(m_xImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(xFormat)
			.setSubresourceRange(xSubresourceRange);

		m_xImageView = xDevice.createImageView(xViewCreate);

		m_xSampler = CreateSampler();
	}

	//just used for depth texture for now
	void VulkanTexture2D::InitWithoutData() {

		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device xDevice = pxRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pxRenderer->GetPhysicalDevice();

		vk::ImageCreateInfo xImageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent({ m_uWidth,m_uHeight,1 })
			.setMipLevels(1)
			.setArrayLayers(1)
			.setFormat(m_xFormat)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		m_xImage = xDevice.createImage(xImageInfo);

		vk::MemoryRequirements xMemRequirements = xDevice.getImageMemoryRequirements(m_xImage);

		uint32_t memoryType = -1;
		for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
			if ((xMemRequirements.memoryTypeBits & (1 << i)) && (xPhysDevice.getMemoryProperties().memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal) {
				memoryType = i;
				break;
			}
		}
		VCE_ASSERT(memoryType != -1, "couldn't find physical memory type");

		vk::MemoryAllocateInfo xAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(xMemRequirements.size)
			.setMemoryTypeIndex(memoryType);

		m_xDeviceMemory = xDevice.allocateMemory(xAllocInfo);

		xDevice.bindImageMemory(m_xImage, m_xDeviceMemory,0);

		vk::ImageSubresourceRange xSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eDepth)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		vk::ImageViewCreateInfo xViewCreate = vk::ImageViewCreateInfo()
			.setImage(m_xImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(m_xFormat)
			.setSubresourceRange(xSubresourceRange);

		m_xImageView = xDevice.createImageView(xViewCreate);
	}

	void VulkanTexture2D::PlatformInit() {
		if (m_bHasFilePath)InitWithData();
		else InitWithoutData();
	}

	vk::Sampler VulkanTexture2D::CreateSampler() {

		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		vk::PhysicalDevice xPhysDevice = VulkanRenderer::GetInstance()->GetPhysicalDevice();


		//todo all these should be parameters
		vk::SamplerCreateInfo xInfo = vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eNearest)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(xPhysDevice.getProperties().limits.maxSamplerAnisotropy)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMaxLod(0);	

		return std::move(xDevice.createSampler(xInfo));


	}

	void VulkanTexture2D::BindToShader(Shader* shader, const std::string& uniformName, uint32_t bindPoint) const
	{
	}

	void VulkanTexture2D::Bind() const
	{
	}

	void VulkanTexture2D::Unbind() const
	{
	}

	uint32_t VulkanTexture2D::GetWidth() const
	{
		return m_uWidth;
	}

	uint32_t VulkanTexture2D::GetHeight() const
	{
		return m_uHeight;
	}
	
}