#pragma once

#include "VeryCoolEngine/Application.h"
#include <glm/glm.hpp>
#include "Block.h"
#include "Chunk.h"

namespace VeryCoolEngine {

	

	class Game : public Application {
	public:
		Game();
		~Game();

		std::vector<Chunk> _chunks;

		std::vector<Block> _blocks;
		std::vector<glm::mat4> _instanceMats;
		std::vector<glm::ivec2> _instanceOffsets;
	};


}
