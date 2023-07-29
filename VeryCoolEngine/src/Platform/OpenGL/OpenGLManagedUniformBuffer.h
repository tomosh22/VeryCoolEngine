#pragma once
#include <glad/glad.h>
#include "VeryCoolEngine/Renderer/ManagedUniformBuffer.h"

namespace VeryCoolEngine {
	class OpenGLManagedUniformBuffer : public ManagedUniformBuffer
	{
	public:
		OpenGLManagedUniformBuffer(uint32_t size, uint8_t numFrames);
		void UploadData(const void* const data, uint32_t size, uint8_t frame, uint32_t offset = 0) override;
	private:
		GLuint* _pUBOs;
	};

}

