#include "Game.h"



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
};
Game::Game() {
}

Game::~Game(){}

//extern definition (EntryPoint.h)
VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
	return new Game();
}