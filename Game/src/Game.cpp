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


		_textures.reserve(100);//#TODO this is just so the textures don't get jigged around in memory when adding new ones
		_textures.push_back(Texture2D::Create("atlas.png", false));

		m_pxBlockFaceMesh = Mesh::GenerateQuad(); 
		m_pxBlockFaceMesh->SetShader(Shader::Create("../Assets/Shaders/vulkan/blockVert.spv", "../Assets/Shaders/vulkan/blockFrag.spv"));
		_meshes.push_back(m_pxBlockFaceMesh);

		DescriptorSpecification xCamSpec;
		xCamSpec.m_aeUniformBufferStages.push_back({&_pCameraUBO, ShaderStageVertexAndFragment });

		DescriptorSpecification xBlockTexSpec;
		xBlockTexSpec.m_aeSamplerStages.push_back({&_textures.back(), ShaderStageFragment});

		m_pxQuadMesh = Mesh::GenerateQuad();
		m_pxQuadMesh->SetShader(Shader::Create("../Assets/Shaders/vulkan/fullscreenVert.spv", "../Assets/Shaders/vulkan/fullscreenFrag.spv"));
		_meshes.push_back(m_pxQuadMesh);

		m_xPipelineSpecs.insert(
			{ "Skybox",
					PipelineSpecification(
					"Skybox",
					m_pxQuadMesh,
					BlendFactor::SrcAlpha,
					BlendFactor::OneMinusSrcAlpha,
					false,
					false,
					DepthCompareFunc::GreaterOrEqual,
					ColourFormat::BGRA8_sRGB,
					DepthFormat::D32_SFloat,
					{xCamSpec},
					&m_pxRenderPass
					)
			});

		m_axPipelineMeshes.insert({ "Skybox", std::vector<Mesh*>() });

		m_axPipelineMeshes.at("Skybox").push_back(m_pxQuadMesh);

		m_xPipelineSpecs.insert(
			{ "Blocks",
					PipelineSpecification(
					"Blocks",
					m_pxBlockFaceMesh,
					BlendFactor::SrcAlpha,
					BlendFactor::OneMinusSrcAlpha,
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					ColourFormat::BGRA8_sRGB,
					DepthFormat::D32_SFloat,
					{ xCamSpec, xBlockTexSpec },
					&m_pxRenderPass
					)
			});

		m_axPipelineMeshes.insert({ "Blocks", std::vector<Mesh*>()});

		m_axPipelineMeshes.at("Blocks").push_back(m_pxBlockFaceMesh);


		m_pxTerrainMesh = Mesh::GenerateGenericHeightmap(10, 10);
		
		_textures.push_back(Texture2D::Create("crystal1k/violet_crystal_43_04_diffuse.jpg", false));
		
		m_pxTerrainMesh->SetShader(Shader::Create("../Assets/Shaders/vulkan/terrainVert.spv", "../Assets/Shaders/vulkan/terrainFrag.spv"));
		_meshes.push_back(m_pxTerrainMesh);
		
		DescriptorSpecification xTerrainTexSpec;
		xTerrainTexSpec.m_aeSamplerStages.push_back({ &_textures.back(), ShaderStageFragment });
		
		m_xPipelineSpecs.insert(
			{ "Terrain",
					PipelineSpecification(
					"Terrain",
					m_pxTerrainMesh,
					BlendFactor::SrcAlpha,
					BlendFactor::OneMinusSrcAlpha,
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					ColourFormat::BGRA8_sRGB,
					DepthFormat::D32_SFloat,
					{xCamSpec, xTerrainTexSpec},
					&m_pxRenderPass
					)
			});
		
		m_axPipelineMeshes.insert({ "Terrain", std::vector<Mesh*>() });
		
		m_axPipelineMeshes.at("Terrain").push_back(m_pxTerrainMesh);

		//#TODO let client set skybox texture

		Chunk::seed = rand();
		GenerateChunks();

		UploadChunks();

		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, 1280.f / 720.f);
		
		
		_renderThreadCanStart = true;
		return;
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

		m_pxBlockFaceMesh->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Float4,
			"_aInstanceQuat",
			false,
			true,
			1,
			_instanceQuats.data(),
			_instanceQuats.size()
		));


		m_pxBlockFaceMesh->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Float3,
			"_aInstancePosition",
			false,
			true,
			1,
			_instancePositions.data(),
			_instancePositions.size()
		));

		m_pxBlockFaceMesh->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Int2,
			"_aInstanceAtlasOffset",
			false,
			true,
			1,
			_instanceOffsets.data(),
			_instanceOffsets.size()
		));

		m_pxBlockFaceMesh->m_axInstanceData.push_back(BufferElement(
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
			game->m_pxBlockFaceMesh->m_axInstanceData.clear();
			game->m_pxBlockFaceMesh->m_uNumInstances = 0;
			game->_instanceMats.clear();
			game->_instanceQuats.clear();
			game->_instancePositions.clear();
			game->_instanceOffsets.clear();
			game->_instanceAOValues.clear();
			game->GenerateChunks();
			
			game->UploadChunks();
			scene->_functionsToRun.push_back([]() {//TODO call this from vulkan, currently leaves a load of glitchy block faces at the origin
				
				Game* game = (Game*)Application::GetInstance();
					game->m_pxBlockFaceMesh->GetVertexArray()->DisableVertexBuffer(game->m_pxBlockFaceMesh->GetVertexArray()->_VertexBuffers.back());
				
					

					VertexBuffer* instancedVertexBuffer = game->m_pxBlockFaceMesh->CreateInstancedVertexBuffer();
					game->m_pxBlockFaceMesh->GetVertexArray()->AddVertexBuffer(instancedVertexBuffer, true);
				
				});

		}
		prevRState = rState;
	}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


