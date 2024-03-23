#include "vcepch.h"
#include "VulkanMemoryManager.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace VeryCoolEngine {
	VulkanMemoryManager::VulkanMemoryManager()
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::PhysicalDevice& xPhysDevice = pxRenderer->GetPhysicalDevice();
		vk::Device& xDevice = pxRenderer->GetDevice();

		uint32_t uCpuMemoryType = ~0u;
		for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
			vk::MemoryType xMemType = xPhysDevice.getMemoryProperties().memoryTypes[i];
			if(xMemType.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible && 
				xMemType.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent &&
				xPhysDevice.getMemoryProperties().memoryHeaps[xMemType.heapIndex].size > g_uCpuPoolSize
				)
				uCpuMemoryType = i;
		}
		VCE_ASSERT(uCpuMemoryType != ~0u, "couldn't find physical memory type");
		

		vk::MemoryAllocateInfo xCpuAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(g_uCpuPoolSize)
			.setMemoryTypeIndex(uCpuMemoryType);

		m_xCPUMemory = xDevice.allocateMemory(xCpuAllocInfo);



		uint32_t uGpuMemoryType = ~0u;
		for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
			vk::MemoryType xMemType = xPhysDevice.getMemoryProperties().memoryTypes[i];
			if (xMemType.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal &&
				xPhysDevice.getMemoryProperties().memoryHeaps[xMemType.heapIndex].size > g_uGpuPoolSize
				)
				uGpuMemoryType = i;
		}
		VCE_ASSERT(uGpuMemoryType != ~0u, "couldn't find physical memory type");


		vk::MemoryAllocateInfo xGpuAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(g_uGpuPoolSize)
			.setMemoryTypeIndex(uGpuMemoryType);

		m_xGPUMemory = xDevice.allocateMemory(xGpuAllocInfo);
	}

	VulkanBuffer* VulkanMemoryManager::AllocateBuffer(size_t uSize, vk::BufferUsageFlags eUsageFlags, MemoryResidency eResidency)
	{
		VulkanBuffer* pxRet = new VulkanBuffer;
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device& xDevice = pxRenderer->GetDevice();

		vk::BufferCreateInfo xBufferInfo = vk::BufferCreateInfo()
			.setSize(uSize)
			.setUsage(eUsageFlags)
			.setSharingMode(vk::SharingMode::eExclusive);
		pxRet->m_xBuffer = xDevice.createBuffer(xBufferInfo);

		vk::MemoryRequirements xRequirements = xDevice.getBufferMemoryRequirements(pxRet->m_xBuffer);
		uint32_t uAlign = xRequirements.alignment;

		if (eResidency == CPU_RESIDENT) {
			if (uNextFreeCpuOffset + uSize >= g_uCpuPoolSize)
				HandleCpuOutOfMemory();

			xDevice.bindBufferMemory(pxRet->m_xBuffer, m_xCPUMemory, uNextFreeCpuOffset);

			MemoryAllocation xAllocation = { uSize, uNextFreeCpuOffset };
			m_xCpuAllocations.push_back(xAllocation);
			m_xCpuAllocationMap.insert({pxRet, xAllocation });
			uNextFreeCpuOffset += uSize;
		}
		else if (eResidency == GPU_RESIDENT) {
			if (ALIGN(uNextFreeGpuOffset, uAlign) + uSize >= g_uGpuPoolSize)
				HandleGpuOutOfMemory();

			xDevice.bindBufferMemory(pxRet->m_xBuffer, m_xGPUMemory, ALIGN(uNextFreeGpuOffset, uAlign));

			MemoryAllocation xAllocation = { uSize, ALIGN(uNextFreeGpuOffset, uAlign) };
			m_xGpuAllocations.push_back(xAllocation);
			m_xGpuAllocationMap.insert({ pxRet, xAllocation });
			uNextFreeGpuOffset = ALIGN(uNextFreeGpuOffset, uAlign) + uSize;
		}

		return pxRet;
	}

	void VulkanMemoryManager::UploadData(void* pAllocation, void* pData, size_t uSize) {
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device& xDevice = pxRenderer->GetDevice();
		auto xCpuIt = m_xCpuAllocationMap.find(pAllocation);
		auto xGpuIt = m_xGpuAllocationMap.find(pAllocation);
		VCE_ASSERT(pData, "Invalid data");
		if (xCpuIt != m_xCpuAllocationMap.end()) {
			VCE_ASSERT(m_xGpuAllocationMap.find(pAllocation) == m_xGpuAllocationMap.end(), "This allocation has somehow become a CPU and GPU allocation???");
			MemoryAllocation xAlloc = xCpuIt->second;
			void* pMap = xDevice.mapMemory(m_xCPUMemory, xAlloc.m_uOffset, xAlloc.m_uSize);
			memcpy(pMap, pData, uSize);
			xDevice.unmapMemory(m_xCPUMemory);
		}
		else if (xGpuIt != m_xGpuAllocationMap.end()) {
			VCE_ASSERT(m_xCpuAllocationMap.find(pAllocation) == m_xCpuAllocationMap.end(), "This allocation has somehow become a CPU and GPU allocation???");
			MemoryAllocation xAlloc = xGpuIt->second;
			VulkanBuffer* pxVkBuffer = reinterpret_cast<VulkanBuffer*>(pAllocation);

			//#TO_TODO: staging pool
			VulkanBuffer pxStagingBuffer = VulkanBuffer(uSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			void* pMappedData = xDevice.mapMemory(pxStagingBuffer.m_xDeviceMem, 0, uSize);
			memcpy(pMappedData, pData, uSize);
			xDevice.unmapMemory(pxStagingBuffer.m_xDeviceMem);
			VulkanBuffer::CopyBufferToBuffer(&pxStagingBuffer, pxVkBuffer, uSize);

		}
		else
			VCE_ASSERT(false, "This allocation didn't go through the memory manager");
	}

	bool VulkanMemoryManager::MemoryWasAllocated(void* pAllocation)
	{
		return m_xCpuAllocationMap.find(pAllocation) != m_xCpuAllocationMap.end() || m_xGpuAllocationMap.find(pAllocation) != m_xGpuAllocationMap.end();
	}

	void VulkanMemoryManager::HandleCpuOutOfMemory() {
		VCE_ASSERT(false, "Implement me");
	}

	void VulkanMemoryManager::HandleGpuOutOfMemory() {
		VCE_ASSERT(false, "Implement me");
	}
}