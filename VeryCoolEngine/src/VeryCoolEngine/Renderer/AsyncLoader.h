#pragma once
#include "VeryCoolEngine/Renderer/RendererAPI.h"

namespace VeryCoolEngine {
	class Texture2D;
	class Material;
	class AsyncLoader {
	public:
		~AsyncLoader() {
			delete g_pxAsyncLoaderCommandBuffer;
		}
		static std::unordered_map<Texture2D*, std::string> g_xPendingStreams;
		static std::vector<Texture2D*> g_xReceivingTextures;
		static std::mutex g_xAsyncLoaderMutex;
		static RendererAPI::CommandBuffer* g_pxAsyncLoaderCommandBuffer;
		static class Buffer* g_pxStagingBuffer;

		void ThreadFunc();

		void ProcessPendingStreams_AsyncLoaderThread();
		void ProcessPendingStreams_MainThread();
	};
}
