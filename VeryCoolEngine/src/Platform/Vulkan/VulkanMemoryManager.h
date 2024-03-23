#pragma once
#include <vulkan/vulkan.hpp>

namespace VeryCoolEngine {
	class VulkanBuffer;

	constexpr size_t g_uCpuPoolSize = 1024u * 1024u * 1024u * 2u;
	constexpr size_t g_uGpuPoolSize = 1024u * 1024u * 1024u * 2u;

	class VulkanMemoryManager
	{
	public:
		
		VulkanMemoryManager();

		enum MemoryResidency : uint8_t {
			CPU_RESIDENT,
			GPU_RESIDENT
		};

		

		VulkanBuffer* AllocateBuffer(uint32_t uSize, vk::BufferUsageFlags eUsageFlags, MemoryResidency eResidency);

		void* MapMemory(void* pAllocation);
		void UnmapMemory(void* pAllocation);

		bool MemoryWasAllocated(void* pAllocation);
	private:

		void HandleCpuOutOfMemory();

		vk::DeviceMemory m_xCPUMemory;
		vk::DeviceMemory m_xGPUMemory;
		
		struct MemoryAllocation {
			size_t m_uSize;
			size_t m_uOffset;
		};
		std::list<MemoryAllocation> m_xCpuAllocations;
		std::list<MemoryAllocation> m_xGpuAllocations;

		std::unordered_map<void*, MemoryAllocation> m_xCpuAllocationMap;
		std::unordered_map<void*, MemoryAllocation> m_xGpuAllocationMap;

		size_t uNextFreeCpuOffset = 0;
		size_t uNextFreeGpuOffset = 0;
	};

}