#include "vcepch.h"
#include "ManagedUniformBuffer.h"
#include "Platform/OpenGL/OpenGLManagedUniformBuffer.h"


namespace VeryCoolEngine {
	ManagedUniformBuffer* ManagedUniformBuffer::Create(uint32_t size, uint8_t numFrames) {
#ifdef VCE_OPENGL
		return new OpenGLManagedUniformBuffer(size, numFrames);
#endif
	}
}