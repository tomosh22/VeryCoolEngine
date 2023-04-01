#include "Game.h"
Game::Game() {
	
}

Game::~Game(){}

//extern definition (EntryPoint.h)
VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
	return new Game();
}