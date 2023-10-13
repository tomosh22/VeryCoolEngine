#pragma once

#include "VeryCoolEngine/Application.h"
#include <glm/glm.hpp>
#include "Block.h"
#include "Chunk.h"
#include <map>

namespace VeryCoolEngine {

	

	class Game : public Application {
	public:
		Game();
		~Game();

		void GenerateChunks();
		void UploadChunks();

		static constexpr glm::ivec3 s_xNumChunks = { 8,1,8 };

		std::map<long long, Chunk*> _chunks;
		Block GetAdjacentBlock(const Chunk* chunk, int x, int y, int z, int offsetX, int offsetY, int offsetZ);

		std::vector<Block> _blocks;
		std::vector<glm::mat4> _instanceMats;
		std::vector<glm::quat> _instanceQuats;
		std::vector<glm::vec3> _instancePositions;
		std::vector<glm::ivec2> _instanceOffsets;
		std::vector<glm::ivec4> _instanceAOValues;

		Mesh* m_pxBlockFaceMesh;

		Chunk* chunkPtrs[s_xNumChunks.x * s_xNumChunks.z];
	};


}
