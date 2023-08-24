#include "Game.h"
#include <imgui.h>


#include "VeryCoolEngine.h"

namespace VeryCoolEngine {

	const std::unordered_map<Block::BlockType, glm::ivec2> Block::atlasOffsets = {
		{ Block::BlockType::Cobblestone, {0,0} },
		{ Block::BlockType::Stone, {1,0} },
		{ Block::BlockType::Dirt, {2,0} },
		{ Block::BlockType::Grass, {3,0} },
		{ Block::BlockType::WoodenPlanks, {4,0} },
		{ Block::BlockType::StoneSlab, {5,0} },
		{ Block::BlockType::PolishedStone, {6,0} },
		{ Block::BlockType::Brick, {7,0} },
		{ Block::BlockType::TNT, {8,0} },
	};

	Game::Game() {
		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 0, 5), 0, 0, 45, 1, 1000, 1280.f / 720.f);
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

		for (size_t x = 0; x < 5; x++)
		{
			for (size_t z = 0; z < 5; z++)
			{
				_chunks.emplace_back(Chunk({ x,0,z }));
			}
		}

		for(Chunk& chunk : _chunks) chunk.UploadVisibleFaces();
		
		_pMesh->_instanceData.push_back(BufferElement (
			ShaderDataType::Mat4,
			"_aInstanceMat",
			false,
			true,
			1,
			_instanceMats.data(),
			_instanceMats.size()
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

	Game::~Game() {}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


