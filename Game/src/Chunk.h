#pragma once
#include "Block.h"
#include <glm/glm.hpp>

namespace VeryCoolEngine {

	typedef long long ChunkKey_t;

	class Chunk
	{
	public:

		

		static glm::ivec3 _chunkSize;

		Chunk() = default;
		Chunk(const glm::ivec3 pos);
		~Chunk() = default;

		void UploadVisibleFaces();
		void UploadFace(Block block, Block::Side side, int x, int y, int z);
		static Transform GetTransformForSide(Block::Side side, const glm::ivec3& position);

		//stores x in leftmost 32 bits and z in rightmost 32 bits
		static ChunkKey_t CalcKey(int x, int z) {return ChunkKey_t(x)<<32|z;}

		//#todo this is a bit lazy
		bool ShouldUploadRight(const Block& block, int x, int y, int z);
		bool ShouldUploadLeft(const Block& block, int x, int y, int z);
		bool ShouldUploadTop(const Block& block, int x, int y, int z);
		bool ShouldUploadBottom(const Block& block, int x, int y, int z);
		bool ShouldUploadFront(const Block& block, int x, int y, int z);
		bool ShouldUploadBack(const Block& block, int x, int y, int z);

		glm::ivec4 GetAOValuesTop(const Block& block, int x, int y, int z);
		glm::ivec4 GetAOValuesFront(const Block& block, int x, int y, int z);
		glm::ivec4 GetAOValuesRight(const Block& block, int x, int y, int z);
		glm::ivec4 GetAOValuesLeft(const Block& block, int x, int y, int z);
		glm::ivec4 GetAOValuesBack(const Block& block, int x, int y, int z);

		Block*** _blocks;//xyz

		glm::ivec3 _chunkPos = { 0,0,0 };
	};

}


