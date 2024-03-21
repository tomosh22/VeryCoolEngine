#pragma once

namespace VeryCoolEngine{
	class ManagedUniformBuffer
	{
	public:
		virtual void UploadData(const void* const data, uint32_t size, uint8_t frame, uint32_t offset = 0) = 0;
		static ManagedUniformBuffer* Create(uint32_t size, uint8_t numFrames, uint32_t baseBinding);

		class Buffer** ppBuffers;
	};
}


