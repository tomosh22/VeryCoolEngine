#include "Game.h"
#include <imgui.h>


#include "VeryCoolEngine.h"

namespace VeryCoolEngine {

	const std::unordered_map<Block::FaceType, glm::ivec2> Block::atlasOffsets = {
		{ Block::FaceType::GrassFull, {0,0} },
		{ Block::FaceType::Stone, {1,0} },
		{ Block::FaceType::Dirt, {2,0} },
		{ Block::FaceType::GrassSide, {3,0} },
		{ Block::FaceType::WoodenPlanks, {4,0} },
		{ Block::FaceType::StoneSlab, {5,0} },
		{ Block::FaceType::PolishedStone, {6,0} },
		{ Block::FaceType::Brick, {7,0} },
		{ Block::FaceType::TNT, {8,0} },
	};

	Game::Game() {
#ifdef VCE_VULKAN
		_pMesh = Mesh::GenerateGenericHeightmap(128, 128);
		return;
#endif
		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, 1280.f / 720.f);
#pragma region old
		//_pMesh = Mesh::GenerateGenericHeightmap(2,2);
		//_pMesh = Mesh::GenerateCubeFace();
		//_pMesh->SetTexture(Texture2D::Create("crystal2k/violet_crystal_43_04_diffuse.jpg", false));
		//_pMesh->SetTexture(Texture2D::Create("atlas.png", false));
		//_pMesh->SetBumpMap(Texture2D::Create("crystal2k/violet_crystal_43_04_normal.jpg", false));

		//_pMesh->SetShader(Shader::Create("basic.vert", "basic.frag"));
#pragma endregion
		//#todo these should probably be in string maps?
		//or do i store them individually?
		_shaders.push_back(Shader::Create("block.vert", "block.frag"));
		_textures.push_back(Texture2D::Create("atlas.png", false));

		_pMesh = Mesh::GenerateCubeFace();
		_pMesh->SetShader(_shaders[0]);
		_pMesh->SetTexture(_textures[0]);

		Chunk::seed = rand();
		GenerateChunks();
		
		UploadChunks();


		_pFullscreenShader = Shader::Create("fullscreen.vert", "fullscreen.frag");

		_pCubemap = TextureCube::Create("CubemapTest", false);

		_lights.push_back({
				500,75,100,100,
				0,1,0,1
			});
		_lights.push_back({
				100,75,100,100,
				0,1,0,1
			});
		_lights.push_back({
			250, 100, 250, 1000,
				0.8, 0.8, 0.8, 1
			});
		_lights.push_back({
			0, 5, 0, 1000,
				0.8, 0.8, 0.8, 1
			});

		

		_renderThreadCanStart = true;
	}

	void Game::GenerateChunks() {
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
				auto func = [](Chunk* chunk, int x, int z) {
					*chunk = Chunk({ x,0,z });
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

	void Game::UploadChunks() {
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

		_pMesh->_instanceData.push_back(BufferElement(
			ShaderDataType::Float4,
			"_aInstanceQuat",
			false,
			true,
			1,
			_instanceQuats.data(),
			_instanceQuats.size()
		));


		_pMesh->_instanceData.push_back(BufferElement(
			ShaderDataType::Float3,
			"_aInstancePosition",
			false,
			true,
			1,
			_instancePositions.data(),
			_instancePositions.size()
		));

		_pMesh->_instanceData.push_back(BufferElement(
			ShaderDataType::Int2,
			"_aInstanceAtlasOffset",
			false,
			true,
			1,
			_instanceOffsets.data(),
			_instanceOffsets.size()
		));

		_pMesh->_instanceData.push_back(BufferElement(
			ShaderDataType::Int4,
			"_ainstanceAOValues",
			false,
			true,
			1,
			_instanceAOValues.data(),
			_instanceAOValues.size()
		));
	}

	Game::~Game() {}

	Block Game::GetAdjacentBlock(const Chunk* chunk, int x, int y, int z, int offsetX, int offsetY, int offsetZ)
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

	//declared in Application.h, defined by game
	void Application::GameLoop() {
		Game* game = (Game*)Application::GetInstance();
		printf("game loop\n");
		bool rState = Input::IsKeyPressed(VCE_KEY_R);
		if (Input::IsKeyPressed(VCE_KEY_R) && prevRState != rState) {
			Chunk::seed = rand();
			game->_chunks.clear();
			game->_pMesh->_instanceData.clear();
			game->_instanceMats.clear();
			game->_instanceQuats.clear();
			game->_instancePositions.clear();
			game->_instanceOffsets.clear();
			game->_instanceAOValues.clear();
			game->GenerateChunks();
			
			game->UploadChunks();
			scene->_functionsToRun.push_back([]() {
				
				Game* game = (Game*)Application::GetInstance();
					game->_pMesh->GetVertexArray()->DisableVertexBuffer(game->_pMesh->GetVertexArray()->_VertexBuffers.back());
				
					

					VertexBuffer* instancedVertexBuffer = game->_pMesh->CreateInstancedVertexBuffer();
					game->_pMesh->GetVertexArray()->AddVertexBuffer(instancedVertexBuffer, true);
				
				});

		}
		prevRState = rState;
	}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


