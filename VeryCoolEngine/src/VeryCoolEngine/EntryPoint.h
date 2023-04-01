#pragma once

#ifdef VCE_WINDOWS

//to be defined by game
//extern VeryCoolEngine::Application* VeryCoolEngine::CreateApplication();

int main(int argc, char** argv) {

	VeryCoolEngine::Log::Init();
	VCE_CORE_WARN("warning");
	VCE_INFO("info");

	VeryCoolEngine::Application* game = VeryCoolEngine::CreateApplication();
	game->Run();
	delete game;
	VeryCoolEngine::Log::Destroy();
}
#else
#error not built for windows
#endif
