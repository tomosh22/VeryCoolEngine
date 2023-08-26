#pragma once

#include <unordered_map>
#include "VeryCoolEngine/Transform.h"

namespace VeryCoolEngine {

	class Block {
	public:
		enum class BlockType {
			Air,
			Grass,
			Cobblestone,
			Stone,
			Dirt,
			WoodenPlanks,
			StoneSlab,
			PolishedStone,
			Brick,
			TNT
		};
		enum class FaceType {
			GrassSide,
			GrassFull,
			Cobblestone,
			Stone,
			Dirt,
			WoodenPlanks,
			StoneSlab,
			PolishedStone,
			Brick,
			TNT
		};

		enum class Side {
			Right,
			Left,
			Top,
			Bottom,
			Front,
			Back
		};

		static FaceType BlockToFace(const Block::BlockType& type, Side side) {
			switch (type) {
			case BlockType::Air:
				//VCE_ASSERT(false, "trying to find face of air block")
				break;
			case BlockType::Grass:
				if (side == Side::Top) return FaceType::GrassFull;
				if (side == Side::Bottom) return FaceType::Dirt;
				return FaceType::GrassSide;
			case BlockType::Cobblestone:
				return FaceType::Cobblestone;
			case BlockType::Stone:
				return FaceType::Stone;
			case BlockType::Dirt://#todo aren't grass and dirt blocks the same???
				return FaceType::Dirt;
			case BlockType::WoodenPlanks:
				return FaceType::WoodenPlanks;
			case BlockType::StoneSlab:
				return FaceType::StoneSlab;
			case BlockType::PolishedStone:
				return FaceType::PolishedStone;
			case BlockType::Brick:
				return FaceType::Brick;
			case BlockType::TNT:
				return FaceType::TNT;
			}

		};

		static const std::unordered_map<FaceType, glm::ivec2> atlasOffsets;
		Block() = default;
		Block(const glm::ivec3& position, BlockType type);
		
		BlockType _blockType;

		Transform posX;
		Transform negX;
		Transform posY;
		Transform negY;
		Transform posZ;
		Transform negZ;
	};

}
