#include "Block.h"
#include "Game.h"

namespace VeryCoolEngine {

	Block::Block(const glm::ivec3& position, BlockType type) : _blockType(type) {

		Game* app = (Game*)Application::GetInstance();


		posX._yaw = 90;
		posX._position = position;
		posX._position.x -= 0.5;
		posX.UpdateRotation();
		posX.UpdateMatrix();
		//app->_instanceMats.push_back(posX._matrix);

		negX._yaw = 270;
		negX._position = position;
		negX._position.x += 0.5;
		negX.UpdateRotation();
		negX.UpdateMatrix();
		//app->_instanceMats.push_back(negX._matrix);

		posY._roll = 270;
		posY._position = position;
		posY._position.y -= 0.5;
		posY.UpdateRotation();
		posY.UpdateMatrix();
		//app->_instanceMats.push_back(posY._matrix);

		negY._roll = 90;
		negY._position = position;
		negY._position.y += 0.5;
		negY.UpdateRotation();
		negY.UpdateMatrix();
		//app->_instanceMats.push_back(negY._matrix);

		posZ._position = position;
		posZ._position.z -= 0.5;
		posZ.UpdateRotation();
		posZ.UpdateMatrix();
		//app->_instanceMats.push_back(posZ._matrix);

		negZ._yaw = 180;
		negZ._position = position;
		negZ._position.z += 0.5;
		negZ.UpdateRotation();
		negZ.UpdateMatrix();
		//app->_instanceMats.push_back(negZ._matrix);

		//app->_instanceOffsets.push_back(atlasOffsets.find(type)->second);

		app->_numInstances += 6;

	}
}

