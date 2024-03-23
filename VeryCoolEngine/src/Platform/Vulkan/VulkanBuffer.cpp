#include "vcepch.h"
#include "VulkanBuffer.h"

namespace VeryCoolEngine {
	VulkanBuffer::VulkanBuffer(vk::DeviceSize uSize, vk::BufferUsageFlags eUsageFlags, vk::MemoryPropertyFlags eMemProperties)
	{
		m_uSize = uSize;
		VulkanRenderer* pRenderer = VulkanRenderer::GetInstance();
		vk::BufferCreateInfo xInfo = vk::BufferCreateInfo()
			.setSize(uSize)
			.setUsage(eUsageFlags)
			.setSharingMode(vk::SharingMode::eExclusive);

		vk::Device xDevice = pRenderer->GetDevice();
		vk::PhysicalDevice xPhysDevice = pRenderer->GetPhysicalDevice();

		m_xBuffer = xDevice.createBuffer(xInfo);

		vk::MemoryRequirements xRequirements = xDevice.getBufferMemoryRequirements(m_xBuffer);

		uint32_t memoryType = ~0u;
		for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
			if ((xRequirements.memoryTypeBits & (1 << i)) && (xPhysDevice.getMemoryProperties().memoryTypes[i].propertyFlags & eMemProperties) == eMemProperties) {
				memoryType = i;
				break;
			}
		}
		VCE_ASSERT(memoryType != ~0u, "couldn't find physical memory type");

		vk::MemoryAllocateInfo xAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(xRequirements.size)
			.setMemoryTypeIndex(memoryType);

		m_xDeviceMem = xDevice.allocateMemory(xAllocInfo);
		xDevice.bindBufferMemory(m_xBuffer, m_xDeviceMem, 0);
	}

	void VulkanBuffer::UploadData(void* pData, uint32_t uSize)
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device& xDevice = pxRenderer->GetDevice();

		void* pMappedPtr;
		//TO_TODO: clean this up once every buffer goes through the memory manager
		if (pxRenderer->m_pxMemoryManager->MemoryWasAllocated(this)) {
			pxRenderer->m_pxMemoryManager->UploadData(this, pData, uSize);
		}
		else {
			vkMapMemory(xDevice, m_xDeviceMem, 0, uSize, 0, &pMappedPtr);
			memcpy(pMappedPtr, pData, uSize);
			vkUnmapMemory(xDevice, m_xDeviceMem);
		}
	}

	void VulkanBuffer::CopyBufferToBuffer(VulkanBuffer* pxSrc, VulkanBuffer* pxDst, size_t uSize)
	{
		
		vk::CommandBuffer xCmd = VulkanRenderer::GetInstance()->BeginSingleUseCmdBuffer();
		

		vk::BufferCopy xCopyRegion(0,0,uSize);
		xCmd.copyBuffer(pxSrc->m_xBuffer, pxDst->m_xBuffer, xCopyRegion);
		

		VulkanRenderer::GetInstance()->EndSingleUseCmdBuffer(xCmd);

	}
	void VulkanBuffer::CopyBufferToImage(VulkanBuffer* pxSrc, VulkanTexture2D* pxDst, bool bAsyncLoader /*= false*/)
	{
		

		vk::ImageSubresourceLayers xSubresource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1);


		vk::BufferImageCopy region = vk::BufferImageCopy()
			.setBufferOffset(0)
			.setBufferRowLength(0)
			.setBufferImageHeight(0)
			.setImageSubresource(xSubresource)
			.setImageOffset({ 0,0,0 })
			.setImageExtent({ pxDst->GetWidth(), pxDst->GetHeight(), 1});

		if (bAsyncLoader) {


			vk::CommandBuffer xCmd = *reinterpret_cast<vk::CommandBuffer*>(AsyncLoader::g_pxAsyncLoaderCommandBuffer->Platform_GetCurrentCmdBuffer());
			xCmd.copyBufferToImage(pxSrc->m_xBuffer, pxDst->m_xImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);
		}
		else {
			vk::CommandBuffer xCmd = VulkanRenderer::GetInstance()->BeginSingleUseCmdBuffer();
			xCmd.copyBufferToImage(pxSrc->m_xBuffer, pxDst->m_xImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);
			VulkanRenderer::GetInstance()->EndSingleUseCmdBuffer(xCmd);
		}

		
	}
	
}