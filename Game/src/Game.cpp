#include "Game.h"

class ExampleLayer : public VeryCoolEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		VCE_INFO("ExampleLayer::Update");
	}

	void OnEvent(VeryCoolEngine::Event& event) override
	{
		
		VCE_TRACE(event.GetName());
	}
};
Game::Game() {
	PushOverlay(new VeryCoolEngine::ImGuiLayer());
}

Game::~Game(){}

//extern definition (EntryPoint.h)
VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
	return new Game();
}