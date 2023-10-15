#include "vcepch.h"
#include "ManagedUniformBuffer.h"
#include "Platform/Vulkan/VulkanManagedUniformBuffer.h"


namespace VeryCoolEngine {
	ManagedUniformBuffer* ManagedUniformBuffer::Create(uint32_t size, uint8_t numFrames, uint32_t baseBinding) {
#ifdef VCE_OPENGL
		return new OpenGLManagedUniformBuffer(size, numFrames, baseBinding);
#elif defined VCE_VULKAN
		return new VulkanManagedUniformBuffer(size, numFrames, baseBinding);
#endif
	}
}