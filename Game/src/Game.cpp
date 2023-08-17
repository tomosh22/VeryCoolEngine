#include "Game.h"

#include <imgui.h>


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


	class ExampleLayer : public Layer
	{
	public:
		ExampleLayer()
			: Layer("Example") {
		}

		void OnUpdate() override {

		}

		void OnEvent(VeryCoolEngine::Event& event) override {


		}
		void OnImGuiRender() override {
		}
	};
	Game::Game() {
		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 0, 5), 0, 0, 45, 1, 1000, 1280.f / 720.f);
		//_pMesh = Mesh::GenerateGenericHeightmap(2,2);
		//_pMesh = Mesh::GenerateCubeFace();
		//_pMesh->SetTexture(Texture2D::Create("crystal2k/violet_crystal_43_04_diffuse.jpg", false));
		//_pMesh->SetTexture(Texture2D::Create("atlas.png", false));
		//_pMesh->SetBumpMap(Texture2D::Create("crystal2k/violet_crystal_43_04_normal.jpg", false));

		//_pMesh->SetShader(Shader::Create("basic.vert", "basic.frag"));

		_shaders.push_back(Shader::Create("basic.vert", "basic.frag"));
		_textures.push_back(Texture2D::Create("atlas.png", false));

		


		_blocks.push_back(Block({0,0,0}, Block::BlockType::Cobblestone));
		_blocks.push_back(Block({1,0,0},Block::BlockType::Stone));
		_blocks.push_back(Block({2,0,0}, Block::BlockType::Dirt));
		_blocks.push_back(Block({3,0,0}, Block::BlockType::Grass));
		_blocks.push_back(Block({4,0,0}, Block::BlockType::WoodenPlanks));
		_blocks.push_back(Block({5,0,0}, Block::BlockType::StoneSlab));
		_blocks.push_back(Block({6,0,0}, Block::BlockType::PolishedStone));
		_blocks.push_back(Block({7,0,0}, Block::BlockType::Brick));
		_blocks.push_back(Block({8,0,0}, Block::BlockType::TNT));


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


