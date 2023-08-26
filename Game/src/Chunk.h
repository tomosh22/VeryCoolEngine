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
		void UploadFace(const Transform& trans, Block::BlockType blockType, Block::Side side);

		//#todo this is a bit lazy
		bool ShouldUploadRight(const Block& block, int x, int y, int z);
		bool ShouldUploadLeft(const Block& block, int x, int y, int z);
		bool ShouldUploadTop(const Block& block, int x, int y, int z);
		bool ShouldUploadBottom(const Block& block, int x, int y, int z);
		bool ShouldUploadFront(const Block& block, int x, int y, int z);
		bool ShouldUploadBack(const Block& block, int x, int y, int z);

		Block*** _blocks;//xyz

		glm::ivec3 _chunkPos = { 0,0,0 };
	};

}


