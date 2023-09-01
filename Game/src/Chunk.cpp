#include "Chunk.h"
#include "VeryCoolEngine/Application.h"
#include "Game.h"

#define DB_PERLIN_IMPL
#include <db_perlin.hpp>

namespace VeryCoolEngine {

	glm::ivec3 Chunk::_chunkSize = glm::ivec3(16, 256, 16);

	Chunk::Chunk(const glm::ivec3 pos) : _chunkPos(pos)
	{
		_blocks = new Block**[Chunk::_chunkSize.x];

		for (int i = 0; i < Chunk::_chunkSize.x; ++i) {
			_blocks[i] = new Block * [Chunk::_chunkSize.y];
			for (int j = 0; j < Chunk::_chunkSize.y; ++j) {
				_blocks[i][j] = new Block[Chunk::_chunkSize.z];
			}
		}

		
		
		for (int x = 0; x < Chunk::_chunkSize.x; x++)
		{
			for (int y = 0; y < Chunk::_chunkSize.y; y++)
			{
				for (int z = 0; z < Chunk::_chunkSize.z; z++)
				{
					glm::ivec3 blockPos = {
						_chunkPos.x * Chunk::_chunkSize.x + x,
						_chunkPos.y * Chunk::_chunkSize.y + y,//#todo probably dont need to bother with y
						_chunkPos.z * Chunk::_chunkSize.z + z
					};
					_blocks[x][y][z] = Block(blockPos, Block::BlockType::Air);
				}
			}
		}

		//#todo make these static somewhere
		int waterLevel = 64;
		int baseStoneLevel = 48;
		int freq = 16;
		int amp = 10;
		float seed = 345349608;

		//#todo can probably do all this in the above 3 deep loop
		for (int x = 0; x < Chunk::_chunkSize.x; x++)
		{
			for (int z = 0; z < Chunk::_chunkSize.z; z++)
			{
				int chunkX = _chunkPos.x * Chunk::_chunkSize.x + x;
				int chunkZ = _chunkPos.z * Chunk::_chunkSize.z + z;

				int surfaceLevel = waterLevel + db::perlin((float)chunkZ / freq, (float)chunkX / freq, seed) * amp;
				for (int y = 0; y < surfaceLevel; y++)
				{
					glm::ivec3 blockPos = {
						   _chunkPos.x * Chunk::_chunkSize.x + x,
						   _chunkPos.y * Chunk::_chunkSize.y + y,//#todo probably dont need to bother with y
						   _chunkPos.z * Chunk::_chunkSize.z + z
					};
					_blocks[x][y][z] = Block(blockPos, Block::BlockType::Dirt);
				}
				glm::ivec3 blockPos = {
						   _chunkPos.x * Chunk::_chunkSize.x + x,
						   _chunkPos.y * Chunk::_chunkSize.y + surfaceLevel,//#todo probably dont need to bother with y
						   _chunkPos.z * Chunk::_chunkSize.z + z
				};
				_blocks[x][surfaceLevel][z] = Block(blockPos, Block::BlockType::Grass);
			}
		}

		//#todo again probably dont need so many loops
		for (int x = 0; x < Chunk::_chunkSize.x; x++)
		{
			for (int z = 0; z < Chunk::_chunkSize.z; z++)
			{
				int chunkX = _chunkPos.x * Chunk::_chunkSize.x + x;
				int chunkZ = _chunkPos.z * Chunk::_chunkSize.z + z;
				int rockLevel = baseStoneLevel + db::perlin((float)chunkZ / freq, (float)chunkX / freq, seed + rand() * 10) * amp;
				for (int y = 0; y < rockLevel; y++)
				{
					glm::ivec3 blockPos = {
						   _chunkPos.x * Chunk::_chunkSize.x + x,
						   _chunkPos.y * Chunk::_chunkSize.y + y,//#todo probably dont need to bother with y
						   _chunkPos.z * Chunk::_chunkSize.z + z
					};
					_blocks[x][y][z] = Block(blockPos, Block::BlockType::Stone);
				}
			}
		}

		//for (int y = 50; y < 65; y++) {
		//	glm::ivec3 blockPos = {
		//				   _chunkPos.x * Chunk::_chunkSize.x + 15,
		//				   _chunkPos.y * Chunk::_chunkSize.y + y,//#todo probably dont need to bother with y
		//				   _chunkPos.z * Chunk::_chunkSize.z + 15
		//	};
		//	_blocks[15][y][15] = Block(blockPos, Block::BlockType::TNT);
		//}

	}

	Transform Chunk::GetTransformForSide(Block::Side side, const glm::ivec3& position) {
		Transform trans;
		trans._position = position;
		switch (side) {
		case Block::Side::Right:
			trans._yaw = 90;
			trans._position.x -= 0.5;
			break;
		case Block::Side::Left:
			trans._yaw = 270;
			trans._position.x += 0.5;
			break;
		case Block::Side::Top:
			trans._pitch = 270;
			trans._position.y -= 0.5;
			break;
		case Block::Side::Bottom:
			trans._pitch = 90;
			trans._position.y += 0.5;
			break;
		case Block::Side::Front:
			trans._position.z -= 0.5;
			break;
		case Block::Side::Back:
			trans._yaw = 180;
			trans._position.z += 0.5;
			break;
		}
		trans.UpdateRotation();
		return trans;
	}

	void Chunk::UploadFace(Block block, Block::Side side, int x, int y, int z){
		Game* game = (Game*)Application::GetInstance();

		Transform trans = GetTransformForSide(side, block._position);

		game->_instanceQuats.push_back(trans._rotationQuat);
		game->_instanceMats.push_back(Transform::RotationMatFromQuat(trans._rotationQuat));
		game->_instancePositions.push_back(trans._position);

		Block::FaceType faceType = Block::BlockToFace(block._blockType, side);

		game->_instanceOffsets.push_back(Block::atlasOffsets.find(faceType)->second);


		switch (side) {
		case Block::Side::Left:
			game->_instanceAOValues.push_back(GetAOValuesLeft(block, x, y, z));
			break;
		case Block::Side::Right:
			game->_instanceAOValues.push_back(GetAOValuesRight(block, x, y, z));
			break;
		case Block::Side::Front:
			game->_instanceAOValues.push_back(GetAOValuesFront(block, x, y, z));
			break;
		case Block::Side::Back:
			game->_instanceAOValues.push_back(GetAOValuesBack(block, x, y, z));
			break;
		case Block::Side::Top:
			game->_instanceAOValues.push_back(GetAOValuesTop(block, x, y, z));
			break;
		case Block::Side::Bottom:
			//#todo implement bottom side AO
			game->_instanceAOValues.push_back(GetAOValuesFront(block, x, y, z));
			break;
		}

		game->_numInstances++;
	}

	glm::ivec4 Chunk::GetAOValuesTop(const Block& block, int x, int y, int z) {
		Game* game = (Game*)Application::GetInstance();
		int frontLeft = 0, frontRight = 0, backLeft = 0, backRight = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (game->GetAdjacentBlock(this, x, y, z,-1, 1, 0)._blockType != Block::BlockType::Air) {
				frontLeft++;
				backLeft++;
			}
			
			if (x < Chunk::_chunkSize.x) {
				if (game->GetAdjacentBlock(this, x, y, z, 1, 1, 0)._blockType != Block::BlockType::Air) {
					frontRight++;
					backRight++;
				}
			}
			if (z >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, 0, 1, -1)._blockType != Block::BlockType::Air) {
					frontLeft++;
					frontRight++;
				}
			}
			if (z < Chunk::_chunkSize.z) {
				if (game->GetAdjacentBlock(this, x, y, z, 0, 1, 1)._blockType != Block::BlockType::Air) {
					backLeft++;
					backRight++;
				}
			}
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (game->GetAdjacentBlock(this, x, y, z,-1, 1, -1)._blockType != Block::BlockType::Air) {
				frontLeft++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,1, 1, -1)._blockType != Block::BlockType::Air) {
				frontRight++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,-1, 1, 1)._blockType != Block::BlockType::Air) {
				backLeft++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,1, 1, 1)._blockType != Block::BlockType::Air) {
				backRight++;
			}
			
		}
#pragma endregion
		return { frontRight,backRight,frontLeft,backLeft };
	}
	glm::ivec4 Chunk::GetAOValuesFront(const Block& block, int x, int y, int z) {
		Game* game = (Game*)Application::GetInstance();
		int topLeft = 0, topRight = 0, bottomLeft = 0, bottomRight = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (game->GetAdjacentBlock(this, x, y, z,-1, 0, 1)._blockType != Block::BlockType::Air) {
				topLeft++;
				bottomLeft++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,1, 0, 1)._blockType != Block::BlockType::Air) {
				topRight++;
				bottomRight++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,0, -1, 1)._blockType != Block::BlockType::Air) {
				bottomLeft++;
				bottomRight++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,0, 1, 1)._blockType != Block::BlockType::Air) {
				topLeft++;
				topRight++;
			}
			
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (game->GetAdjacentBlock(this, x, y, z,-1, -1, 1)._blockType != Block::BlockType::Air) {
				bottomLeft++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,1, -1, 1)._blockType != Block::BlockType::Air) {
				bottomRight++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,-1, 1, 1)._blockType != Block::BlockType::Air) {
				topLeft++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z,1, 1, 1)._blockType != Block::BlockType::Air) {
				topRight++;
			}
			
		}
#pragma endregion
		return { topRight,bottomRight,topLeft,bottomLeft };
	}
	glm::ivec4 Chunk::GetAOValuesRight(const Block& block, int x, int y, int z) {
		Game* game = (Game*)Application::GetInstance();
		int topFront = 0, topBack = 0, bottomFront = 0, bottomBack = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (game->GetAdjacentBlock(this, x, y, z, 1, 0, -1)._blockType !=Block::BlockType::Air) {
				topFront++;
				bottomFront++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z, 1, 0, 1)._blockType !=Block::BlockType::Air) {
				topBack++;
				bottomBack++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z, 1, -1, 0)._blockType !=Block::BlockType::Air) {
				bottomFront++;
				bottomBack++;
			}
			
			if (game->GetAdjacentBlock(this, x, y, z, 1, 1, 0)._blockType !=Block::BlockType::Air) {
				topFront++;
				topBack++;
			}
			
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
				if (game->GetAdjacentBlock(this, x, y, z, 1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomFront++;
				}
			
				if (game->GetAdjacentBlock(this, x, y, z, 1, -1, 1)._blockType != Block::BlockType::Air) {
					bottomBack++;
				}
			
				if (game->GetAdjacentBlock(this, x, y, z, 1, 1, -1)._blockType != Block::BlockType::Air) {
					topFront++;
				}
			
				if (game->GetAdjacentBlock(this, x, y, z, 1, 1, 1)._blockType != Block::BlockType::Air) {
					topBack++;
				}
			
		}
#pragma endregion
		return { topFront,bottomFront,topBack,bottomBack };
	}
	glm::ivec4 Chunk::GetAOValuesLeft(const Block& block, int x, int y, int z) {
		Game* game = (Game*)Application::GetInstance();
		int topFront = 0, topBack = 0, bottomFront = 0, bottomBack = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (z >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, 0, -1)._blockType != Block::BlockType::Air) {
					topFront++;
					bottomFront++;
				}
			}
			if (z < Chunk::_chunkSize.z) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, 0, 1)._blockType != Block::BlockType::Air) {
					topBack++;
					bottomBack++;
				}
			}
			if (y >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, -1, 0)._blockType != Block::BlockType::Air) {
					bottomFront++;
					bottomBack++;
				}
			}
			if (y < Chunk::_chunkSize.y) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, 1, 0)._blockType != Block::BlockType::Air) {
					topFront++;
					topBack++;
				}
			}
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (z >= 0 && y >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomFront++;
				}
			}
			if (z < Chunk::_chunkSize.z && y >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, -1, 1)._blockType != Block::BlockType::Air) {
					bottomBack++;
				}
			}
			if (z >= 0 && y < Chunk::_chunkSize.y) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, 1, -1)._blockType != Block::BlockType::Air) {
					topFront++;
				}
			}
			if (z < Chunk::_chunkSize.z && y < Chunk::_chunkSize.y) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, 1, 1)._blockType != Block::BlockType::Air) {
					topBack++;
				}
			}
		}
#pragma endregion
		return { topBack,bottomBack,topFront,bottomFront };
	}
	glm::ivec4 Chunk::GetAOValuesBack(const Block& block, int x, int y, int z) {
		Game* game = (Game*)Application::GetInstance();
		int topLeft = 0, topRight = 0, bottomLeft = 0, bottomRight = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (x >= 0) {
				if (game->GetAdjacentBlock(this,x,y,z,-1,0,-1)._blockType != Block::BlockType::Air) {
					topLeft++;
					bottomLeft++;
				}
			}
			if (x < Chunk::_chunkSize.x) {
				if (game->GetAdjacentBlock(this, x, y, z, 1, 0, -1)._blockType != Block::BlockType::Air) {
					topRight++;
					bottomRight++;
				}
			}
			if (y >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, 0, -1, -1)._blockType != Block::BlockType::Air) {
					bottomLeft++;
					bottomRight++;
				}
			}
			if (y < Chunk::_chunkSize.y) {
				if (game->GetAdjacentBlock(this, x, y, z, 0, 1, -1)._blockType != Block::BlockType::Air) {
					topLeft++;
					topRight++;
				}
			}
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (x >= 0 && y >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomLeft++;
				}
			}
			if (x < Chunk::_chunkSize.x && y >= 0) {
				if (game->GetAdjacentBlock(this, x, y, z, 1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomRight++;
				}
			}
			if (x >= 0 && y < Chunk::_chunkSize.y) {
				if (game->GetAdjacentBlock(this, x, y, z, -1, 1, -1)._blockType != Block::BlockType::Air) {
					topLeft++;
				}
			}
			if (x < Chunk::_chunkSize.x && y < Chunk::_chunkSize.y) {
				if (game->GetAdjacentBlock(this, x, y, z, 1, 1, -1)._blockType != Block::BlockType::Air) {
					topRight++;
				}
			}
		}
#pragma endregion
		return { topLeft,bottomLeft,topRight,bottomRight };
	}

#pragma region FaceCheckFunctions
	bool Chunk::ShouldUploadRight(const Block& block, int x, int y, int z) {
		if (x == Chunk::_chunkSize.x-1)return true;
		if (_blocks[x+1][y][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadLeft(const Block& block, int x, int y, int z) {
		if (x == 0)return true;
		if (_blocks[x-1][y][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadTop(const Block& block,int x, int y, int z) {
		if (y == Chunk::_chunkSize.y-1)return true;
		if (_blocks[x][y + 1][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadBottom(const Block& block, int x, int y, int z) {
		if (y == 0)return true;
		if (_blocks[x][y - 1][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadFront(const Block& block, int x, int y, int z) {
		if (z == Chunk::_chunkSize.z-1)return true;
		if (_blocks[x][y][z+1]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadBack(const Block& block, int x, int y, int z) {
		if (z == 0)return true;
		if (_blocks[x][y][z-1]._blockType == Block::BlockType::Air)return true;
		return false;
	}
#pragma endregion
	
	void Chunk::UploadVisibleFaces()
	{
		//#todo should probably cache this
		Game* game = (Game*)Application::GetInstance();

		for (int x = 0; x < Chunk::_chunkSize.x; x++)
		{
			for (int y = 0; y < Chunk::_chunkSize.y; y++)
			{
				for (int z = 0; z < Chunk::_chunkSize.z; z++)
				{
					Block block = _blocks[x][y][z];

					//air doesn't have any faces
					if (block._blockType == Block::BlockType::Air)continue;

					if (ShouldUploadRight(block, x, y, z)) UploadFace(block, Block::Side::Right, x, y, z);
					if (ShouldUploadLeft(block, x, y, z)) UploadFace(block, Block::Side::Left, x, y, z);
					if (ShouldUploadTop(block, x, y, z))UploadFace(block, Block::Side::Top, x, y, z);
					if (ShouldUploadBottom(block, x, y, z)) UploadFace(block, Block::Side::Bottom, x, y, z);
					if (ShouldUploadFront(block, x, y, z)) UploadFace(block, Block::Side::Front, x, y, z);
					if (ShouldUploadBack(block, x, y, z)) UploadFace(block, Block::Side::Back, x, y, z);
				}
			}
		}

	}
}

