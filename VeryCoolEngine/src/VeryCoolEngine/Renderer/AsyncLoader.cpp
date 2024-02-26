#include "vcepch.h"
#include "AsyncLoader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	std::unordered_map<Texture2D*, std::string> AsyncLoader::g_xPendingStreams;
	std::vector<Texture2D*> AsyncLoader::g_xReceivingTextures;
	std::mutex AsyncLoader::g_xAsyncLoaderMutex;

	void AsyncLoader::ThreadFunc() {
		Application* pxApp = Application::GetInstance();
		while (pxApp->_running) {
			
		}
	}

	void AsyncLoader::ProcessPendingStreams_AsyncLoaderThread(){
		g_xAsyncLoaderMutex.lock();
		if (g_xPendingStreams.size()) {
			Texture2D* pxTex = g_xPendingStreams.begin()->first;
			std::string pxFile = g_xPendingStreams.begin()->second;

			Texture2D* pxNewTex = Texture2D::Create(pxFile, TextureStreamPriority::NotStreamed);
			pxNewTex->PlatformInit();

			pxTex->m_xStreamInfo.pxNewTex = pxNewTex;
			g_xPendingStreams.erase(g_xPendingStreams.begin());
			g_xReceivingTextures.push_back(pxTex);
		}
		g_xAsyncLoaderMutex.unlock();

		
	}

	void AsyncLoader::ProcessPendingStreams_MainThread() {
		g_xAsyncLoaderMutex.lock();
		if (g_xReceivingTextures.size()) {
			for (auto it = g_xReceivingTextures.begin(); it != g_xReceivingTextures.end(); it++) {
				(*it)->ReceiveStream();
				if((*it)->m_pxParentMaterial)
					(*it)->m_pxParentMaterial->HandleStreamUpdate();
			}
			g_xReceivingTextures.clear();
		}
		g_xAsyncLoaderMutex.unlock();
	}
}