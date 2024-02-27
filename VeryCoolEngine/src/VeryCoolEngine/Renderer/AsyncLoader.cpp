#include "vcepch.h"
#include "AsyncLoader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
#include "VeryCoolEngine/Application.h"
#ifdef VCE_VULKAN
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#endif
#include "stb_image.h"
#include "VeryCoolEngine/AssetHandling/Assets.h"

namespace VeryCoolEngine {
	std::unordered_map<Texture2D*, std::string> AsyncLoader::g_xPendingStreams;
	std::vector<Texture2D*> AsyncLoader::g_xReceivingTextures;
	std::mutex AsyncLoader::g_xAsyncLoaderMutex;
	RendererAPI::CommandBuffer* AsyncLoader::g_pxAsyncLoaderCommandBuffer = nullptr;
	Buffer* AsyncLoader::g_pxStagingBuffer = nullptr;

	void AsyncLoader::ThreadFunc() {
#ifdef VCE_VULKAN
		g_pxAsyncLoaderCommandBuffer = new VulkanCommandBuffer(true);
#endif
		g_pxStagingBuffer = Buffer::CreateStaging(1024u * 1024u * 32);
		Application* pxApp = Application::GetInstance();
		while (pxApp->_running) {
			ProcessPendingStreams_AsyncLoaderThread();
		}
	}

	void AsyncLoader::ProcessPendingStreams_AsyncLoaderThread(){
		
		if (g_xPendingStreams.size()) {
			g_xAsyncLoaderMutex.lock();
			Texture2D* pxTex = g_xPendingStreams.begin()->first;
			std::string pxFile = g_xPendingStreams.begin()->second;

			Texture2D* pxNewTex = Texture2D::Create(pxFile, TextureStreamPriority::NotStreamed);
			int iNumChannels;
			pxTex->m_xStreamInfo.m_pNewData = (char*)stbi_load((TEXTUREDIR + pxNewTex->_filePath).c_str(), &pxTex->m_xStreamInfo.m_uNewWidth, &pxTex->m_xStreamInfo.m_uNewHeight, &iNumChannels, STBI_rgb_alpha);

			//#TO_TODO: stop hardcoding size
			g_pxStagingBuffer->UploadData(pxTex->m_xStreamInfo.m_pNewData, pxTex->m_xStreamInfo.m_uNewWidth * pxTex->m_xStreamInfo.m_uNewHeight * 4);

			pxTex->m_xStreamInfo.m_pxNewTex = pxNewTex;
			g_xPendingStreams.erase(g_xPendingStreams.begin());
			
			g_xReceivingTextures.push_back(pxTex);
			g_xAsyncLoaderMutex.unlock();
		}
		

		
	}

	void AsyncLoader::ProcessPendingStreams_MainThread() {
		g_xAsyncLoaderMutex.lock();
		AsyncLoader::g_pxAsyncLoaderCommandBuffer->BeginRecording();
		if (g_xReceivingTextures.size()) {
			for (auto it = g_xReceivingTextures.begin(); it != g_xReceivingTextures.end(); it++) {
				(*it)->ReceiveStream();
				if((*it)->m_pxParentMaterial)
					(*it)->m_pxParentMaterial->HandleStreamUpdate();
			}
			g_xReceivingTextures.clear();
		}
		AsyncLoader::g_pxAsyncLoaderCommandBuffer->EndRecording(RENDER_ORDER_MEMORY_UPDATE, false);
		g_xAsyncLoaderMutex.unlock();
	}
}