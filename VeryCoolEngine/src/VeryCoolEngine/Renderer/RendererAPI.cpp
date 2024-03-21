#include "vcepch.h"
#include "RendererAPI.h"
#ifdef VCE_VULKAN
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#endif

namespace VeryCoolEngine {
	RendererAPI::CommandBuffer* RendererAPI::CommandBuffer::Create(bool bAsyncLoader)
	{
		return new VulkanCommandBuffer(bAsyncLoader);
	}
}


