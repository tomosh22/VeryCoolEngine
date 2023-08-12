#include "Game.h"

#include <imgui.h>

namespace VeryCoolEngine {
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
		_pMesh = Mesh::GenerateGenericHeightmap(100, 100);
		_pMesh->SetTexture(Texture2D::Create("crystal2k/violet_crystal_43_04_diffuse.jpg", false));
		_pMesh->SetBumpMap(Texture2D::Create("crystal2k/violet_crystal_43_04_normal.jpg", false));

		_renderThreadCanStart = true;
	}

	Game::~Game() {}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


