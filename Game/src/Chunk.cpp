#include "Chunk.h"

namespace VeryCoolEngine {
	Chunk::Chunk()
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
					_blocks[x][y][z] = Block({x,y,z}, Block::BlockType::Grass);
				}
			}
		}
	}
}

