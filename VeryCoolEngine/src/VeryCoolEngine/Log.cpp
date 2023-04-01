#include "Log.h"

namespace VeryCoolEngine {

	spdlog::logger* Log::_coreLogger;
	spdlog::logger* Log::_clientLogger;

	void Log::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");
		_coreLogger = spdlog::stdout_color_mt("VeryCoolEngine").get();
		_clientLogger = spdlog::stdout_color_mt("Game").get();

		_coreLogger->set_level(spdlog::level::trace);
		_clientLogger->set_level(spdlog::level::trace);
	}

	void Log::Destroy() {
		delete _coreLogger;
		delete _clientLogger;
	}
}