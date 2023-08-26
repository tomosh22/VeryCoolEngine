#include "Block.h"
#include "Game.h"

namespace VeryCoolEngine {

	Block::Block(const glm::ivec3& position, BlockType type) : _blockType(type), _position(position) {

		//Game* app = (Game*)Application::GetInstance();
		//
		//
		//posX._yaw = 90;
		//posX._position = position;
		//posX._position.x -= 0.5;
		//posX.UpdateRotation();
		//
		//negX._yaw = 270;
		//negX._position = position;
		//negX._position.x += 0.5;
		//negX.UpdateRotation();
		//
		//posY._pitch = 270;
		//posY._position = position;
		//posY._position.y -= 0.5;
		//posY.UpdateRotation();
		//
		//negY._pitch = 90;
		//negY._position = position;
		//negY._position.y += 0.5;
		//negY.UpdateRotation();
		//
		//posZ._position = position;
		//posZ._position.z -= 0.5;
		//posZ.UpdateRotation();
		//
		//negZ._yaw = 180;
		//negZ._position = position;
		//negZ._position.z += 0.5;
		//negZ.UpdateRotation();


	}
}

