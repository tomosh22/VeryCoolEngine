#include "Chunk.h"
#include "VeryCoolEngine/Application.h"
#include "Game.h"

#define DB_PERLIN_IMPL
#include <db_perlin.hpp>

namespace VeryCoolEngine {
	Chunk::Chunk(const glm::ivec3 pos) : _chunkPos(pos)
	{
		_blocks = new Block**[16];

		for (int i = 0; i < 16; ++i) {
			_blocks[i] = new Block * [256];
			for (int j = 0; j < 256; ++j) {
				_blocks[i][j] = new Block[16];
			}
		}

		

		for (int x = 0; x < 16; x++)
		{
			for (int y = 0; y < 256; y++)
			{
				for (int z = 0; z < 16; z++)
				{
					glm::ivec3 blockPos = {
						_chunkPos.x * 16 + x,
						_chunkPos.y * 255 + y,//#todo probably dont need to bother with y
						_chunkPos.z * 16 + z
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
		for (int x = 0; x < 16; x++)
		{
			for (int z = 0; z < 16; z++)
			{
				int chunkX = _chunkPos.x * 16 + x;
				int chunkZ = _chunkPos.z * 16 + z;

				int surfaceLevel = waterLevel + db::perlin((float)chunkZ / freq, (float)chunkX / freq, seed) * amp;
				for (int y = 0; y < surfaceLevel; y++)
				{
					glm::ivec3 blockPos = {
						   _chunkPos.x * 16 + x,
						   _chunkPos.y * 255 + y,//#todo probably dont need to bother with y
						   _chunkPos.z * 16 + z
					};
					_blocks[x][y][z] = Block(blockPos, Block::BlockType::Dirt);
				}
				glm::ivec3 blockPos = {
						   _chunkPos.x * 16 + x,
						   _chunkPos.y * 255 + surfaceLevel,//#todo probably dont need to bother with y
						   _chunkPos.z * 16 + z
				};
				_blocks[x][surfaceLevel][z] = Block(blockPos, Block::BlockType::Grass);
			}
		}

		//#todo again probably dont need so many loops
		for (int x = 0; x < 16; x++)
		{
			for (int z = 0; z < 16; z++)
			{
				int chunkX = _chunkPos.x * 16 + x;
				int chunkZ = _chunkPos.z * 16 + z;
				int rockLevel = baseStoneLevel + db::perlin((float)chunkZ / freq, (float)chunkX / freq, seed + rand() * 10) * amp;
				for (int y = 0; y < rockLevel; y++)
				{
					glm::ivec3 blockPos = {
						   _chunkPos.x * 16 + x,
						   _chunkPos.y * 255 + y,//#todo probably dont need to bother with y
						   _chunkPos.z * 16 + z
					};
					_blocks[x][y][z] = Block(blockPos, Block::BlockType::Stone);
				}
			}
		}

	}

	void Chunk::UploadFace(const Transform& trans, Block::BlockType blockType, Block::Side side){
		Game* game = (Game*)Application::GetInstance();

		game->_instanceQuats.push_back(trans._rotationQuat);
		game->_instanceMats.push_back(Transform::RotationMatFromQuat(trans._rotationQuat));
		game->_instancePositions.push_back(trans._position);

		Block::FaceType faceType = Block::BlockToFace(blockType, side);

		game->_instanceOffsets.push_back(Block::atlasOffsets.find(faceType)->second);
		game->_numInstances++;
	}

	bool Chunk::ShouldUploadRight(const Block& block, int x, int y, int z) {
		if (x == 15)return true;
		if (_blocks[x+1][y][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadLeft(const Block& block, int x, int y, int z) {
		if (x == 0)return true;
		if (_blocks[x-1][y][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadTop(const Block& block,int x, int y, int z) {
		if (y == 255)return true;
		if (_blocks[x][y + 1][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadBottom(const Block& block, int x, int y, int z) {
		if (y == 0)return true;
		if (_blocks[x][y - 1][z]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadFront(const Block& block, int x, int y, int z) {
		if (z == 15)return true;
		if (_blocks[x][y][z+1]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	bool Chunk::ShouldUploadBack(const Block& block, int x, int y, int z) {
		if (z == 0)return true;
		if (_blocks[x][y][z-1]._blockType == Block::BlockType::Air)return true;
		return false;
	}
	
	void Chunk::UploadVisibleFaces()
	{
		//#todo should probably cache this
		Game* game = (Game*)Application::GetInstance();

		for (int x = 0; x < 16; x++)
		{
			for (int y = 0; y < 256; y++)
			{
				for (int z = 0; z < 16; z++)
				{
					Block block = _blocks[x][y][z];

					//air doesn't have any faces
					if (block._blockType == Block::BlockType::Air)continue;

					if (ShouldUploadRight(block, x, y, z)) UploadFace(block.posX, block._blockType, Block::Side::Right);
					if (ShouldUploadLeft(block, x, y, z)) UploadFace(block.negX, block._blockType, Block::Side::Left);
					if (ShouldUploadTop(block, x, y, z)) {
						if(block._blockType == Block::BlockType::Grass)
						UploadFace(block.posY, block._blockType, Block::Side::Top);
					}
					if (ShouldUploadBottom(block, x, y, z)) UploadFace(block.negY, block._blockType, Block::Side::Bottom);
					if (ShouldUploadFront(block, x, y, z)) UploadFace(block.posZ, block._blockType, Block::Side::Front);
					if (ShouldUploadBack(block, x, y, z)) UploadFace(block.negZ, block._blockType, Block::Side::Back);
				}
			}
		}

	}
}

