#pragma once
#include "vcepch.h"

//#TODO move these to pch, cba dealing with build time right now
#include <glm/glm.hpp>
#include <map>

#include "Block.h"

namespace VeryCoolEngine {

	

	class BlockWorld
	{
	public:
		std::map<long long, class Chunk*> _chunks;
		static constexpr glm::ivec3 s_xNumChunks = { 8,1,8 };
		class Chunk* chunkPtrs[s_xNumChunks.x * s_xNumChunks.z];

		std::vector<Block> _blocks;
		std::vector<glm::mat4> _instanceMats;
		std::vector<glm::quat> _instanceQuats;
		std::vector<glm::vec3> _instancePositions;
		std::vector<glm::ivec2> _instanceOffsets;
		std::vector<glm::ivec4> _instanceAOValues;

		BlockWorld();

		void GenerateChunks();
		void UploadChunks();
		Block GetAdjacentBlock(const class Chunk* chunk, int x, int y, int z, int offsetX, int offsetY, int offsetZ);
	};

}