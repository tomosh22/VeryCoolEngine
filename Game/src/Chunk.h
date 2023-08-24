#pragma once
#include "Block.h"
#include <glm/glm.hpp>

namespace VeryCoolEngine {
	class Chunk
	{
	public:
		Chunk() = default;
		Chunk(const glm::ivec3 pos);
		~Chunk() = default;

		void UploadVisibleFaces();

		Block*** _blocks;//xyz

		glm::ivec3 _chunkPos = { 0,0,0 };
	};

}


