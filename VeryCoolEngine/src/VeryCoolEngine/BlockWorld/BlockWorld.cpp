#include "vcepch.h"
#include "BlockWorld.h"
#include "Chunk.h"

#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {

	BlockWorld::BlockWorld() {
		Application* app = Application::GetInstance();

		BufferDescriptorSpecification xCamSpec;
		xCamSpec.m_aeUniformBufferStages.push_back({ &app->_pCameraUBO, ShaderStageVertexAndFragment });

		BufferDescriptorSpecification xLightSpec;

		TextureDescriptorSpecification xBlockTexSpec;
		xBlockTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });

		app->m_pxInstanceMesh = Mesh::GenerateQuad();
		app->m_pxInstanceMesh->SetTexture(Texture2D::Create("atlas.png", false));
		app->m_pxInstanceMesh->SetShader(Shader::Create("vulkan/blockVert.spv", "vulkan/blockFrag.spv"));
		app->m_pxInstanceMesh->m_xTexDescSpec = xBlockTexSpec;
		app->_meshes.push_back(app->m_pxInstanceMesh);

		Chunk::seed = rand();
		GenerateChunks();

		UploadChunks();


		


		

		
		app->m_xPipelineSpecs.insert(
			{ "Blocks",
					PipelineSpecification(
					"Blocks",
					app->m_pxInstanceMesh,
					Shader::Create("vulkan/blockVert.spv", "vulkan/blockFrag.spv"),
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					{ xCamSpec},
					{xBlockTexSpec},
					&app->m_pxRenderPass,
					false,
					false
					)
			});
	}

	void BlockWorld::GenerateChunks() {
		std::thread threads[s_xNumChunks.x * s_xNumChunks.z];


		for (int x = 0; x < s_xNumChunks.x; x++) {
			for (int z = 0; z < s_xNumChunks.z; z++) {
				int index = s_xNumChunks.z * x + z;
				Chunk* chunk = (Chunk*)malloc(sizeof(Chunk));

				//x is first 32 bits, z is second 32 bits
				ChunkKey_t key = Chunk::CalcKey(x, z);

				//std::cout << key << std::endl;
				std::pair<ChunkKey_t, Chunk*> pair = std::make_pair(key, chunk);
				_chunks.emplace(pair);
				auto func = [this](Chunk* chunk, int x, int z) {
					*chunk = Chunk({ x,0,z }, this);
					};
				threads[index] = std::thread(func, chunk, x, z);


				chunkPtrs[index] = chunk;
			}
		}

		for (int x = 0; x < s_xNumChunks.x; x++)
		{
			for (int z = 0; z < s_xNumChunks.z; z++)
			{
				int i = s_xNumChunks.z * x + z;
				threads[i].join();
			}

		}
	}

	void BlockWorld::UploadChunks() {
		for (int x = 0; x < s_xNumChunks.x; x++)
		{
			for (int z = 0; z < s_xNumChunks.z; z++)
			{
				long long key = Chunk::CalcKey(x, z);
				Chunk chunk = *(_chunks.find(key))->second;
				chunk.UploadVisibleFaces();
				for (int x = 0; x < Chunk::_chunkSize.x; x++)
				{
					for (int y = 0; y < Chunk::_chunkSize.y; y++)
					{
						//delete[] chunk._blocks[x][y];

					}
					//delete[] chunk._blocks[x];
				}
				//delete[] chunk._blocks;
			}

		}


		//for (int i = 0; i < s_xNumChunks.z * s_xNumChunks.x; i++) free(chunkPtrs[i]);

		//std::cout << "unique quats " << Transform::uniqueQuats.size() << '\n';

		Application* app = Application::GetInstance();

		app->m_pxInstanceMesh->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Float4,
			"_aInstanceQuat",
			false,
			true,
			1,
			_instanceQuats.data(),
			_instanceQuats.size()
		));


		app->m_pxInstanceMesh->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Float3,
			"_aInstancePosition",
			false,
			true,
			1,
			_instancePositions.data(),
			_instancePositions.size()
		));

		app->m_pxInstanceMesh->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Int2,
			"_aInstanceAtlasOffset",
			false,
			true,
			1,
			_instanceOffsets.data(),
			_instanceOffsets.size()
		));

		app->m_pxInstanceMesh->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Int4,
			"_ainstanceAOValues",
			false,
			true,
			1,
			_instanceAOValues.data(),
			_instanceAOValues.size()
		));
	}

	Block BlockWorld::GetAdjacentBlock(const Chunk* chunk, int x, int y, int z, int offsetX, int offsetY, int offsetZ)
	{
		VCE_ASSERT(abs(offsetX) <= 1 && abs(offsetY) <= 1 && abs(offsetX) <= 1, "can't check more than one block away");

		auto withinChunkFunc = [&](int x, int y, int z) {
			return
				x >= 0 && x < Chunk::_chunkSize.x &&
				y >= 0 && y < Chunk::_chunkSize.y &&
				z >= 0 && z < Chunk::_chunkSize.z;
			};

		int newX = x + offsetX, newY = y + offsetY, newZ = z + offsetZ;
		if (withinChunkFunc(newX, newY, newZ)) return chunk->_blocks[newX][newY][newZ];
		if (newY < 0 || newY >= Chunk::_chunkSize.y) { Block block; block._blockType = Block::BlockType::Air; return block; }

		int chunkOffsetX = (newX >= 0 && newX < Chunk::_chunkSize.x) ? 0 : (newX < 0 ? -1 : 1);
		int chunkOffsetZ = (newZ >= 0 && newZ < Chunk::_chunkSize.z) ? 0 : (newZ < 0 ? -1 : 1);

		int keyX = chunk->_chunkPos.x + chunkOffsetX;
		int keyZ = chunk->_chunkPos.z + chunkOffsetZ;

		ChunkKey_t key = Chunk::CalcKey(keyX, keyZ);
		if (_chunks.contains(key)) {
			int indexX = (chunkOffsetX == 0) ? newX : (chunkOffsetX == -1 ? Chunk::_chunkSize.x - 1 : 0);
			int indexZ = (chunkOffsetZ == 0) ? newZ : (chunkOffsetZ == -1 ? Chunk::_chunkSize.z - 1 : 0);

			Chunk* newChunk = _chunks.find(key)->second;
			Block block = newChunk->_blocks[indexX][newY][indexZ];
			return block;
		}

		Block block;
		block._blockType = Block::BlockType::Air;
		return block;
	}
}