#include "vcepch.h"
#include "Chunk.h"
#include "VeryCoolEngine/Application.h"

#define DB_PERLIN_IMPL
#include <db_perlin.hpp>

#include "BlockWorld.h"

namespace VeryCoolEngine {

	float Chunk::seed = rand();

	glm::ivec3 Chunk::_chunkSize = glm::ivec3(16, 256, 16);

	Chunk::Chunk(const glm::ivec3 pos, class BlockWorld* pxParentWorld) : _chunkPos(pos), m_pxParentWorld(pxParentWorld)
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

	}

	Transform Chunk::GetTransformForSide(Block::Side side, const glm::ivec3& position) {
		Transform trans;
		trans.m_xPosition = position;
		switch (side) {
		case Block::Side::Right:
			trans.m_fYaw = 90;
			trans.m_xPosition.x -= 0.5;
			break;
		case Block::Side::Left:
			trans.m_fYaw = 270;
			trans.m_xPosition.x += 0.5;
			break;
		case Block::Side::Top:
			trans.m_fPitch = 270;
			trans.m_xPosition.y -= 0.5;
			break;
		case Block::Side::Bottom:
			trans.m_fPitch = 90;
			trans.m_xPosition.y += 0.5;
			break;
		case Block::Side::Front:
			trans.m_xPosition.z -= 0.5;
			break;
		case Block::Side::Back:
			trans.m_fYaw = 180;
			trans.m_xPosition.z += 0.5;
			break;
		}
		trans.UpdateRotation();
		return trans;
	}

	void Chunk::UploadFace(Block block, Block::Side side, int x, int y, int z){

		Transform trans = GetTransformForSide(side, block._position);

		m_pxParentWorld->_instanceQuats.push_back(trans.m_xRotationQuat);
		m_pxParentWorld->_instanceMats.push_back(Transform::RotationMatFromQuat(trans.m_xRotationQuat));
		m_pxParentWorld->_instancePositions.push_back(trans.m_xPosition);

		Block::FaceType faceType = Block::BlockToFace(block._blockType, side);

		m_pxParentWorld->_instanceOffsets.push_back(Block::atlasOffsets.find(faceType)->second);


		switch (side) {
		case Block::Side::Left:
			m_pxParentWorld->_instanceAOValues.push_back(GetAOValuesLeft(block, x, y, z));
			break;
		case Block::Side::Right:
			m_pxParentWorld->_instanceAOValues.push_back(GetAOValuesRight(block, x, y, z));
			break;
		case Block::Side::Front:
			m_pxParentWorld->_instanceAOValues.push_back(GetAOValuesFront(block, x, y, z));
			break;
		case Block::Side::Back:
			m_pxParentWorld->_instanceAOValues.push_back(GetAOValuesBack(block, x, y, z));
			break;
		case Block::Side::Top:
			m_pxParentWorld->_instanceAOValues.push_back(GetAOValuesTop(block, x, y, z));
			break;
		case Block::Side::Bottom:
			//#todo implement bottom side AO
			m_pxParentWorld->_instanceAOValues.push_back(GetAOValuesFront(block, x, y, z));
			break;
		}

		Application::GetInstance()->m_pxInstanceMesh->m_uNumInstances++;
	}

	glm::ivec4 Chunk::GetAOValuesTop(const Block& block, int x, int y, int z) {
		int frontLeft = 0, frontRight = 0, backLeft = 0, backRight = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,-1, 1, 0)._blockType != Block::BlockType::Air) {
				frontLeft++;
				backLeft++;
			}
			
			if (x < Chunk::_chunkSize.x) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 1, 0)._blockType != Block::BlockType::Air) {
					frontRight++;
					backRight++;
				}
			}
			if (z >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, 1, -1)._blockType != Block::BlockType::Air) {
					frontLeft++;
					frontRight++;
				}
			}
			if (z < Chunk::_chunkSize.z) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, 1, 1)._blockType != Block::BlockType::Air) {
					backLeft++;
					backRight++;
				}
			}
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,-1, 1, -1)._blockType != Block::BlockType::Air) {
				frontLeft++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,1, 1, -1)._blockType != Block::BlockType::Air) {
				frontRight++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,-1, 1, 1)._blockType != Block::BlockType::Air) {
				backLeft++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,1, 1, 1)._blockType != Block::BlockType::Air) {
				backRight++;
			}
			
		}
#pragma endregion
		return { frontRight,backRight,frontLeft,backLeft };
	}
	glm::ivec4 Chunk::GetAOValuesFront(const Block& block, int x, int y, int z) {
		int topLeft = 0, topRight = 0, bottomLeft = 0, bottomRight = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,-1, 0, 1)._blockType != Block::BlockType::Air) {
				topLeft++;
				bottomLeft++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,1, 0, 1)._blockType != Block::BlockType::Air) {
				topRight++;
				bottomRight++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,0, -1, 1)._blockType != Block::BlockType::Air) {
				bottomLeft++;
				bottomRight++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,0, 1, 1)._blockType != Block::BlockType::Air) {
				topLeft++;
				topRight++;
			}
			
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,-1, -1, 1)._blockType != Block::BlockType::Air) {
				bottomLeft++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,1, -1, 1)._blockType != Block::BlockType::Air) {
				bottomRight++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,-1, 1, 1)._blockType != Block::BlockType::Air) {
				topLeft++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z,1, 1, 1)._blockType != Block::BlockType::Air) {
				topRight++;
			}
			
		}
#pragma endregion
		return { topRight,bottomRight,topLeft,bottomLeft };
	}
	glm::ivec4 Chunk::GetAOValuesRight(const Block& block, int x, int y, int z) {
		int topFront = 0, topBack = 0, bottomFront = 0, bottomBack = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 0, -1)._blockType !=Block::BlockType::Air) {
				topFront++;
				bottomFront++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 0, 1)._blockType !=Block::BlockType::Air) {
				topBack++;
				bottomBack++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, -1, 0)._blockType !=Block::BlockType::Air) {
				bottomFront++;
				bottomBack++;
			}
			
			if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 1, 0)._blockType !=Block::BlockType::Air) {
				topFront++;
				topBack++;
			}
			
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomFront++;
				}
			
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, -1, 1)._blockType != Block::BlockType::Air) {
					bottomBack++;
				}
			
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 1, -1)._blockType != Block::BlockType::Air) {
					topFront++;
				}
			
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 1, 1)._blockType != Block::BlockType::Air) {
					topBack++;
				}
			
		}
#pragma endregion
		return { topFront,bottomFront,topBack,bottomBack };
	}
	glm::ivec4 Chunk::GetAOValuesLeft(const Block& block, int x, int y, int z) {
		int topFront = 0, topBack = 0, bottomFront = 0, bottomBack = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (z >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, 0, -1)._blockType != Block::BlockType::Air) {
					topFront++;
					bottomFront++;
				}
			}
			if (z < Chunk::_chunkSize.z) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, 0, 1)._blockType != Block::BlockType::Air) {
					topBack++;
					bottomBack++;
				}
			}
			if (y >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, -1, 0)._blockType != Block::BlockType::Air) {
					bottomFront++;
					bottomBack++;
				}
			}
			if (y < Chunk::_chunkSize.y) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, 1, 0)._blockType != Block::BlockType::Air) {
					topFront++;
					topBack++;
				}
			}
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (z >= 0 && y >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomFront++;
				}
			}
			if (z < Chunk::_chunkSize.z && y >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, -1, 1)._blockType != Block::BlockType::Air) {
					bottomBack++;
				}
			}
			if (z >= 0 && y < Chunk::_chunkSize.y) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, 1, -1)._blockType != Block::BlockType::Air) {
					topFront++;
				}
			}
			if (z < Chunk::_chunkSize.z && y < Chunk::_chunkSize.y) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, 1, 1)._blockType != Block::BlockType::Air) {
					topBack++;
				}
			}
		}
#pragma endregion
		return { topBack,bottomBack,topFront,bottomFront };
	}
	glm::ivec4 Chunk::GetAOValuesBack(const Block& block, int x, int y, int z) {
		int topLeft = 0, topRight = 0, bottomLeft = 0, bottomRight = 0;
		bool checkSides = true;
		bool checkCorners = true;
#pragma region sides
		if (checkSides) {
			if (x >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this,x,y,z,-1,0,-1)._blockType != Block::BlockType::Air) {
					topLeft++;
					bottomLeft++;
				}
			}
			if (x < Chunk::_chunkSize.x) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 0, -1)._blockType != Block::BlockType::Air) {
					topRight++;
					bottomRight++;
				}
			}
			if (y >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, -1, -1)._blockType != Block::BlockType::Air) {
					bottomLeft++;
					bottomRight++;
				}
			}
			if (y < Chunk::_chunkSize.y) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, 1, -1)._blockType != Block::BlockType::Air) {
					topLeft++;
					topRight++;
				}
			}
		}
#pragma endregion
#pragma region corners
		if (checkCorners) {
			if (x >= 0 && y >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomLeft++;
				}
			}
			if (x < Chunk::_chunkSize.x && y >= 0) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, -1, -1)._blockType != Block::BlockType::Air) {
					bottomRight++;
				}
			}
			if (x >= 0 && y < Chunk::_chunkSize.y) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, 1, -1)._blockType != Block::BlockType::Air) {
					topLeft++;
				}
			}
			if (x < Chunk::_chunkSize.x && y < Chunk::_chunkSize.y) {
				if (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 1, -1)._blockType != Block::BlockType::Air) {
					topRight++;
				}
			}
		}
#pragma endregion
		return { topLeft,bottomLeft,topRight,bottomRight };
	}

#pragma region FaceCheckFunctions
	bool Chunk::ShouldUploadRight(const Block& block, int x, int y, int z) {
		return (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 1, 0, 0)._blockType == Block::BlockType::Air);
	}
	bool Chunk::ShouldUploadLeft(const Block& block, int x, int y, int z) {
		return (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, -1, 0, 0)._blockType == Block::BlockType::Air);
	}
	bool Chunk::ShouldUploadTop(const Block& block,int x, int y, int z) {
		return (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, 1, 0)._blockType == Block::BlockType::Air);
	}
	bool Chunk::ShouldUploadBottom(const Block& block, int x, int y, int z) {
		return (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, -1, 0)._blockType == Block::BlockType::Air);
	}
	bool Chunk::ShouldUploadFront(const Block& block, int x, int y, int z) {
		return (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, 0, 1)._blockType == Block::BlockType::Air);
	}
	bool Chunk::ShouldUploadBack(const Block& block, int x, int y, int z) {
		return (m_pxParentWorld->GetAdjacentBlock(this, x, y, z, 0, 0, -1)._blockType == Block::BlockType::Air);
	}
#pragma endregion
	
	void Chunk::UploadVisibleFaces()
	{

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

