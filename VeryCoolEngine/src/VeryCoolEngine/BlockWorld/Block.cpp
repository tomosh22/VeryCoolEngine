#include "vcepch.h"

#include "Block.h"

namespace VeryCoolEngine {

	Block::Block(const glm::ivec3& position, BlockType type) : _blockType(type), _position(position) {}
}

