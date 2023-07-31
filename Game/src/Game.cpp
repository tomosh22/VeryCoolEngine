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
	}

	Game::~Game() {}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


