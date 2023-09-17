#include "vcepch.h"
#include "ManagedUniformBuffer.h"
#include "Platform/OpenGL/OpenGLManagedUniformBuffer.h"


namespace VeryCoolEngine {
	ManagedUniformBuffer* ManagedUniformBuffer::Create(uint32_t size, uint8_t numFrames, uint32_t baseBinding) {
#ifdef VCE_OPENGL
		return new OpenGLManagedUniformBuffer(size, numFrames, baseBinding);
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
}