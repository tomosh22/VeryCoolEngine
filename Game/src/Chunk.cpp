#include "Chunk.h"
#include "VeryCoolEngine/Application.h"
#include "Game.h"

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
					_blocks[x][y][z] = Block(blockPos, Block::BlockType::Grass);
				}
			}
		}
	}

	void Chunk::UploadFace(const Transform& trans, Block::BlockType type) {
		Game* game = (Game*)Application::GetInstance();

		game->_instanceMats.push_back(trans._matrix);
		game->_instanceOffsets.push_back(Block::atlasOffsets.find(type)->second);
		game->_numInstances++;
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
					if (x == 15) {
						UploadFace(block.posX, block._blockType);
					}
					if (x == 0) {
						UploadFace(block.negX, block._blockType);
					}
					if (y == 255) {
						UploadFace(block.posY, block._blockType);
					}
					if (y == 0) {
						UploadFace(block.negY, block._blockType);
					}
					if (z == 15) {
						UploadFace(block.posZ, block._blockType);
					}
					if (z == 0) {
						UploadFace(block.negZ, block._blockType);
					}
				}
			}
		}

	}
}

