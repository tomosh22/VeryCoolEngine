#pragma once

#include <unordered_map>
#include "VeryCoolEngine/Transform.h"

namespace VeryCoolEngine {

	class Block {
	public:
		enum class BlockType {
			Cobblestone,
			Stone,
			Dirt,
			Grass,
			WoodenPlanks,
			StoneSlab,
			PolishedStone,
			Brick,
			TNT
		};
		static const std::unordered_map<BlockType, glm::ivec2> atlasOffsets;
		Block() = default;
		Block(const glm::vec3& position, BlockType type);
		
		BlockType _blockType;

		Transform posX;
		Transform negX;
		Transform posY;
		Transform negY;
		Transform posZ;
		Transform negZ;
	};

}
