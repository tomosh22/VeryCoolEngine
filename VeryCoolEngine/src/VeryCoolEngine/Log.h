#pragma once
#include "core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <memory>


#define VCE_CORE_TRACE(...) VeryCoolEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VCE_CORE_INFO(...) VeryCoolEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VCE_CORE_WARN(...) VeryCoolEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VCE_CORE_ERROR(...) VeryCoolEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define VCE_CORE_FATAL(...) VeryCoolEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define VCE_TRACE(...) VeryCoolEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VCE_INFO(...) VeryCoolEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define VCE_WARN(...) VeryCoolEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VCE_ERROR(...) VeryCoolEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define VCE_FATAL(...) VeryCoolEngine::Log::GetClientLogger()->fatal(__VA_ARGS__)

namespace VeryCoolEngine {
	class VCE_API Log
	{
	public:
		static void Init();
		static void Destroy();

		inline static spdlog::logger* GetCoreLogger() { return _coreLogger; }
		inline static spdlog::logger* GetClientLogger() { return _clientLogger; }
	private:
		static spdlog::logger* _coreLogger;
		static spdlog::logger* _clientLogger;
	};
}
