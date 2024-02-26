#pragma once

namespace VeryCoolEngine {
	class Texture2D;
	class AsyncLoader {
	public:
		static std::unordered_map<Texture2D*, std::string> g_xPendingStreams;
		static std::vector<Texture2D*> g_xReceivingTextures;
		static std::mutex g_xAsyncLoaderMutex;

		void ThreadFunc();

		void ProcessPendingStreams_AsyncLoaderThread();
		void ProcessPendingStreams_MainThread();
	};
}
