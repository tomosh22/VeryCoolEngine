#pragma once
#include <string>
#ifdef VCE_WINDOWS


//to be defined by game
//extern VeryCoolEngine::Application* VeryCoolEngine::CreateApplication();

int main(int argc, char** argv) {

	VeryCoolEngine::Log::Init();

	VeryCoolEngine::Application* game = VeryCoolEngine::CreateApplication();
	game->Run();
	VeryCoolEngine::Log::Destroy();
	delete game;
	
}
#else
#error not built for windows
#endif
