#include "Game.h"

#include <imgui.h>

class ExampleLayer : public VeryCoolEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example"){
	}

	void OnUpdate() override{
		
	}

	void OnEvent(VeryCoolEngine::Event& event) override{
		
		
	}
	void OnImGuiRender() override {
		ImGui::Begin("Test");
		ImGui::Text("Test");
		float test[4]{};
		ImGui::ColorEdit4("hi" ,test);
		ImGui::End();
	}
};
Game::Game() {
	PushOverlay(new ExampleLayer());
}

Game::~Game(){}

//extern definition (EntryPoint.h)
VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
	return new Game();
}