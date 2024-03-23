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

		vk::MemoryRequirements xCpuRequirements;

		vk::MemoryPropertyFlags eMemProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		uint32_t memoryType = ~0u;
		for (uint32_t i = 0; i < xPhysDevice.getMemoryProperties().memoryTypeCount; i++) {
			vk::MemoryType xMemType = xPhysDevice.getMemoryProperties().memoryTypes[i];
			if(xMemType.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible && 
				xMemType.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent &&
				xPhysDevice.getMemoryProperties().memoryHeaps[xMemType.heapIndex].size > g_uCpuPoolSize
				)
				memoryType = i;
		}
		VCE_ASSERT(memoryType != ~0u, "couldn't find physical memory type");
		
		xCpuRequirements.size = g_uCpuPoolSize;
		xCpuRequirements.alignment = 1;
		xCpuRequirements.memoryTypeBits = memoryType;
		

		vk::MemoryAllocateInfo xAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(xCpuRequirements.size)
			.setMemoryTypeIndex(memoryType);

		m_xCPUMemory = xDevice.allocateMemory(xAllocInfo);
	}

	VulkanBuffer* VulkanMemoryManager::AllocateBuffer(uint32_t uSize, vk::BufferUsageFlags eUsageFlags, MemoryResidency eResidency)
	{
		VulkanBuffer* pxRet = new VulkanBuffer;
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device& xDevice = pxRenderer->GetDevice();

		vk::BufferCreateInfo xBufferInfo = vk::BufferCreateInfo()
			.setSize(uSize)
			.setUsage(eUsageFlags)
			.setSharingMode(vk::SharingMode::eExclusive);
		pxRet->m_xBuffer = xDevice.createBuffer(xBufferInfo);

		if (eResidency == CPU_RESIDENT) {
			if (uNextFreeCpuOffset + uSize >= g_uCpuPoolSize)
				HandleCpuOutOfMemory();

			xDevice.bindBufferMemory(pxRet->m_xBuffer, m_xCPUMemory, uNextFreeCpuOffset);

			MemoryAllocation xAllocation = { uSize, uNextFreeCpuOffset };
			m_xCpuAllocations.push_back(xAllocation);
			m_xCpuAllocationMap.insert({pxRet, xAllocation });
			uNextFreeCpuOffset += uSize;
		}

		return pxRet;
	}

	void* VulkanMemoryManager::MapMemory(void* pAllocation)
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device& xDevice = pxRenderer->GetDevice();
		auto xCpuIt = m_xCpuAllocationMap.find(pAllocation);
		if (xCpuIt != m_xCpuAllocationMap.end()) {
			VCE_ASSERT(m_xGpuAllocationMap.find(pAllocation) == m_xGpuAllocationMap.end(), "This allocation has somehow become a CPU and GPU allocation???");
			MemoryAllocation xAlloc = xCpuIt->second;
			return xDevice.mapMemory(m_xCPUMemory, xAlloc.m_uOffset, xAlloc.m_uSize);
		}
		
	}

	void VulkanMemoryManager::UnmapMemory(void* pAllocation)
	{
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		vk::Device& xDevice = pxRenderer->GetDevice();
		auto xCpuIt = m_xCpuAllocationMap.find(pAllocation);
		if (xCpuIt != m_xCpuAllocationMap.end()) {
			VCE_ASSERT(m_xGpuAllocationMap.find(pAllocation) == m_xGpuAllocationMap.end(), "This allocation has somehow become a CPU and GPU allocation???");
			xDevice.unmapMemory(m_xCPUMemory);
		}

	}

	bool VulkanMemoryManager::MemoryWasAllocated(void* pAllocation)
	{
		return m_xCpuAllocationMap.find(pAllocation) != m_xCpuAllocationMap.end() || m_xGpuAllocationMap.find(pAllocation) != m_xGpuAllocationMap.end();
	}

	void VulkanMemoryManager::HandleCpuOutOfMemory() {
		VCE_ASSERT(false, "Implement me");
	}
}