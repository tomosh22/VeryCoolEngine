#include "vcepch.h"
#include "VulkanTexture.h"
#ifdef VCE_VULKAN
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>
#include "VeryCoolEngine/AssetHandling/Assets.h"
#include "VulkanBuffer.h"
#include "VeryCoolEngine/Renderer/AsyncLoader.h"
#include "Platform/Vulkan/VulkanAssetHandler.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace VeryCoolEngine {

	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureWrapMode wrapMode) {
	
		m_uWidth = width;
		m_uHeight = height;
		//TODO: switch case
		if(format == TextureFormat::D)
			m_xFormat = vk::Format::eD32Sfloat;
		if (format == TextureFormat::RGBA)
			m_xFormat = vk::Format::eR8G8B8A8Unorm;
	}

	VulkanTexture2D::VulkanTexture2D(const std::string& path, TextureStreamPriority eStreamPrio, bool srgb) {
		m_bHasFilePath = true;
		_filePath = path;
		_srgb = srgb;
		m_xStreamInfo.m_ePrio = eStreamPrio;

	}

	VulkanTexture2D::~VulkanTexture2D() {
		//#TO_TODO: can do this in PlatformInit
		stbi_image_free(m_pData);

		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		xDevice.destroySampler(m_xSampler);
		xDevice.destroyImageView(m_xImageView);
		xDevice.destroyImage(m_xImage);
		xDevice.freeMemory(m_xDeviceMemory);
	}

	void VulkanTexture2D::InitWithFileName(bool bAsyncLoader /*= false*/) {
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device xDevice = pxRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pxRenderer->GetPhysicalDevice();

		vk::Format xFormat = vk::Format::eR8G8B8A8Srgb;//#todo handle different formats

		int uWidth, uHeight, uNumChannels;

		if(m_xStreamInfo.m_ePrio == TextureStreamPriority::NotStreamed)
			m_pData = (char*)stbi_load((TEXTUREDIR + _filePath).c_str(), &uWidth, &uHeight, &uNumChannels, STBI_rgb_alpha);
		else {
			std::string strOriginalPath = _filePath;
			_filePath = _filePath.replace(_filePath.find("."), 0, "_lowres");
			m_pData = (char*)stbi_load((TEXTUREDIR + _filePath).c_str(), &uWidth, &uHeight, &uNumChannels, STBI_rgb_alpha);
			AsyncLoader::g_xPendingStreams.insert({this, strOriginalPath });
		}

		m_uWidth = uWidth; m_uHeight = uHeight;

		m_uNumMips = std::floor(std::log2(std::max(m_uWidth, m_uHeight))) + 1;

		//#TO_TODO: stop hardcoding
		vk::DeviceSize uSize = uWidth * uHeight * 4;

		VulkanBuffer* pxStagingBuffer = new VulkanBuffer(uSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		pxStagingBuffer->UploadData(m_pData, uSize);

		vk::ImageCreateInfo xImageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(xFormat)
			.setTiling(vk::ImageTiling::eOptimal)
			.setExtent({ m_uWidth,m_uHeight,1 })
			.setMipLevels(m_uNumMips)
			.setArrayLayers(1)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc)
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

		if (bAsyncLoader) {
			for (uint32_t i = 0; i < m_uNumMips; i++)
				pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, i, 0, true);
			VulkanBuffer::CopyBufferToImage(pxStagingBuffer, this, true);
			pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, 0, 0, true);
			for (uint32_t i = 1; i < m_uNumMips; i++)
				BlitImageToImage(this, this, i, true);
			pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, 0, 0, true);
			for (uint32_t i = 1; i < m_uNumMips; i++)
				pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, i, 0, true);//todo will probably need to change this to vertex shader in the future

		}
		else {
			VulkanCommandBuffer* pxBlockingCmdBuffer = dynamic_cast<VulkanAssetHandler*>(Application::GetInstance()->m_pxAssetHandler)->m_pxBlockingCommandBuffer;

			for (uint32_t i = 0; i < m_uNumMips; i++)
				pxBlockingCmdBuffer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, i, 0);
			pxBlockingCmdBuffer->CopyBufferToImage(pxStagingBuffer, this);
			pxBlockingCmdBuffer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, 0, 0);
			for (uint32_t i = 1; i < m_uNumMips; i++)
				pxBlockingCmdBuffer->BlitImageToImage(this, this, i);
			pxBlockingCmdBuffer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, 0, 0);
			for (uint32_t i = 1; i < m_uNumMips; i++)
				pxBlockingCmdBuffer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, i, 0);//todo will probably need to change this to vertex shader in the future
		}
		
		//#TO_TODO: delete staging buffer
		//delete pxStagingBuffer;


		vk::ImageSubresourceRange xSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(m_uNumMips)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		vk::ImageViewCreateInfo xViewCreate = vk::ImageViewCreateInfo()
			.setImage(m_xImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(xFormat)
			.setSubresourceRange(xSubresourceRange);

		m_xImageView = xDevice.createImageView(xViewCreate);

		m_xSampler = CreateSampler();

#ifdef VCE_DEBUG
		m_bInitialised = true;
#endif
	}

	void VulkanTexture2D::InitWithData() {
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device xDevice = pxRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pxRenderer->GetPhysicalDevice();

		vk::Format xFormat = vk::Format::eR8G8B8A8Srgb;//#todo handle different formats

		int uWidth = 0, uHeight = 0, uNumChannels;

		struct aiTexel {
			unsigned char b, g, r, a;
		} texel = *reinterpret_cast<aiTexel*>(m_pData);

		stbi_uc* data = stbi_load_from_memory((stbi_uc*)m_pData, m_uDataLength,&uWidth,&uHeight,&uNumChannels, STBI_rgb_alpha);

		VCE_ASSERT(uWidth != 0 && uHeight != 0, "width and height not set by stbi");

		m_uWidth = uWidth; m_uHeight = uHeight;

		vk::DeviceSize uSize = uWidth * uHeight * 4;

		VulkanBuffer* pxStagingBuffer = new VulkanBuffer(uSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		pxStagingBuffer->UploadData(data, uSize);

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

		pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer);
		VulkanBuffer::CopyBufferToImage(pxStagingBuffer, this);
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

#ifdef VCE_DEBUG
		m_bInitialised = true;
#endif
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
			.setUsage(m_bIsDepthTexture ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eSampled)
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
			.setAspectMask(m_bIsDepthTexture ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor)
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

		if(!m_bIsDepthTexture) m_xSampler = CreateSampler();

#ifdef VCE_DEBUG
		m_bInitialised = true;
#endif
	}

	void VulkanTexture2D::PlatformInit(bool bAsyncLoader /*= false*/) {
		if (m_bInitialised)return;
		if (m_bHasFilePath)InitWithFileName(bAsyncLoader);
		else if (m_pData)InitWithData();
		else InitWithoutData();
	}

	void VulkanTexture2D::ReceiveStream()
	{

		//#TO_TODO: can do this in PlatformInit
		stbi_image_free(m_pData);

		

		VulkanTexture2D* pxVkTex = dynamic_cast<VulkanTexture2D*>(m_xStreamInfo.m_pxNewTex);
		m_uWidth = m_xStreamInfo.m_uNewWidth;
		m_uHeight = m_xStreamInfo.m_uNewHeight;

		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device xDevice = pxRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pxRenderer->GetPhysicalDevice();

		xDevice.waitIdle();

		//xDevice.destroySampler(m_xSampler);
		xDevice.destroyImageView(m_xImageView);
		xDevice.destroyImage(m_xImage);
		xDevice.freeMemory(m_xDeviceMemory);


		vk::ImageCreateInfo xImageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setTiling(vk::ImageTiling::eOptimal)
			.setExtent({ m_uWidth,m_uHeight,1 })
			.setMipLevels(m_uNumMips)
			.setArrayLayers(1)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc)
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

		for (uint32_t i = 0; i < m_uNumMips; i++)
			pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, i, 0, true);
		VulkanBuffer::CopyBufferToImage(dynamic_cast<VulkanBuffer*>(AsyncLoader::g_pxStagingBuffer), this, true);
		pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, 0, 0, true);
		for (uint32_t i = 1; i < m_uNumMips; i++)
			BlitImageToImage(this, this, i, true);
		pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, 0, 0, true);
		for (uint32_t i = 1; i < m_uNumMips; i++)
			pxRenderer->ImageTransitionBarrier(m_xImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, i, 0, true);//todo will probably need to change this to vertex shader in the future

		vk::ImageSubresourceRange xSubresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(m_uNumMips)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		vk::ImageViewCreateInfo xViewCreate = vk::ImageViewCreateInfo()
			.setImage(m_xImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setSubresourceRange(xSubresourceRange);

		m_xImageView = xDevice.createImageView(xViewCreate);
	}

	vk::Sampler VulkanTexture2D::CreateSampler() {

		vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
		vk::PhysicalDevice xPhysDevice = VulkanRenderer::GetInstance()->GetPhysicalDevice();


		//todo all these should be parameters
		vk::SamplerCreateInfo xInfo = vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(xPhysDevice.getProperties().limits.maxSamplerAnisotropy)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMaxLod(FLT_MAX);	

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
	
	VulkanTexture2D* VulkanTexture2D::CreateVulkanTexture2D(uint32_t uWidth, uint32_t uHeight, uint32_t uMipCount, vk::Format eFormat, vk::ImageAspectFlags eAspect, vk::ImageUsageFlags eUsage, vk::ImageLayout eLayout, vk::PipelineStageFlags ePipeType){


		VulkanTexture2D* pxTex = new VulkanTexture2D();
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device xDevice = pxRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pxRenderer->GetPhysicalDevice();

		vk::ImageCreateInfo xImageCreateInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(uWidth, uHeight, 1))
			.setFormat(eFormat)
			.setUsage(eUsage)
			.setMipLevels(uMipCount)
			.setArrayLayers(1)
			.setImageType(vk::ImageType::e2D);

		pxTex->m_xImage = xDevice.createImage(xImageCreateInfo);

		vk::MemoryRequirements xMemRequirements = xDevice.getImageMemoryRequirements(pxTex->m_xImage);

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

		pxTex->m_xDeviceMemory = xDevice.allocateMemory(xAllocInfo);

		xDevice.bindImageMemory(pxTex->m_xImage, pxTex->m_xDeviceMemory, 0);

		vk::ImageViewCreateInfo xViewCreateInfo = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(eFormat)
			.setSubresourceRange(vk::ImageSubresourceRange(eAspect, 0,uMipCount, 0, 1))
			.setImage(pxTex->m_xImage);

		pxTex->m_xImageView = xDevice.createImageView(xViewCreateInfo);

		pxTex->m_xSampler = CreateSampler();

		vk::CommandBuffer xCmd = pxRenderer->BeginSingleUseCmdBuffer();

		pxRenderer->ImageTransitionBarrier(pxTex->m_xImage, vk::ImageLayout::eUndefined, eLayout, eAspect, vk::PipelineStageFlagBits::eTopOfPipe, ePipeType);

		pxTex->m_uWidth = uWidth;
		pxTex->m_uHeight = uHeight;

		return pxTex;
	}

	VulkanTexture2D* VulkanTexture2D::CreateColourAttachment(uint32_t uWidth, uint32_t uHeight, uint32_t uMipCount, vk::Format eFormat) {
		return CreateVulkanTexture2D(uWidth, uHeight, uMipCount, eFormat, vk::ImageAspectFlagBits::eColor, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput);
	}
	VulkanTexture2D* VulkanTexture2D::CreateDepthAttachment(uint32_t uWidth, uint32_t uHeight) {
		return CreateVulkanTexture2D(uWidth, uHeight, 1, vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageLayout::eDepthAttachmentOptimal, vk::PipelineStageFlagBits::eEarlyFragmentTests);
	}
	void VulkanTexture2D::BlitImageToImage(VulkanTexture2D* pxSrc, VulkanTexture2D* pxDst, uint32_t uDstMip, bool bAsyncLoader /*= false*/)
	{
		std::array<vk::Offset3D, 2> axSrcOffsets;
		axSrcOffsets.at(0).setX(0);
		axSrcOffsets.at(0).setY(0);
		axSrcOffsets.at(0).setZ(0);
		axSrcOffsets.at(1).setX(pxSrc->GetWidth());
		axSrcOffsets.at(1).setY(pxSrc->GetHeight());
		axSrcOffsets.at(1).setZ(1);

		vk::ImageSubresourceLayers xSrcSubresource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		std::array<vk::Offset3D, 2> axDstOffsets;
		axDstOffsets.at(0).setX(0);
		axDstOffsets.at(0).setY(0);
		axDstOffsets.at(0).setZ(0);
		axDstOffsets.at(1).setX(pxSrc->GetWidth() / std::pow(2, uDstMip));
		axDstOffsets.at(1).setY(pxSrc->GetHeight() / std::pow(2, uDstMip));
		axDstOffsets.at(1).setZ(1);

		vk::ImageSubresourceLayers xDstSubresource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(uDstMip)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		vk::ImageBlit xBlit = vk::ImageBlit()
			.setSrcOffsets(axSrcOffsets)
			.setDstOffsets(axDstOffsets)
			.setSrcSubresource(xSrcSubresource)
			.setDstSubresource(xDstSubresource);

		if (bAsyncLoader) {

			vk::CommandBuffer xCmd = *reinterpret_cast<vk::CommandBuffer*>(AsyncLoader::g_pxAsyncLoaderCommandBuffer->Platform_GetCurrentCmdBuffer());
			xCmd.blitImage(pxSrc->m_xImage, vk::ImageLayout::eTransferSrcOptimal, pxDst->m_xImage, vk::ImageLayout::eTransferDstOptimal, xBlit, vk::Filter::eLinear);
		}
		else {
			vk::CommandBuffer xCmd = VulkanRenderer::GetInstance()->BeginSingleUseCmdBuffer();
			xCmd.blitImage(pxSrc->m_xImage, vk::ImageLayout::eTransferSrcOptimal, pxDst->m_xImage, vk::ImageLayout::eTransferDstOptimal, xBlit, vk::Filter::eLinear);
			VulkanRenderer::GetInstance()->EndSingleUseCmdBuffer(xCmd);
		}
	}
}