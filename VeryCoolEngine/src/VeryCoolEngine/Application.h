#pragma once

#include "core.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Window.h"
#include "VeryCoolEngine/Layers/LayerStack.h"
#include "VeryCoolEngine/Events/Event.h"
#include "VeryCoolEngine/Events/ApplicationEvent.h"
#include "VeryCoolEngine/ImGui/ImGuiLayer.h"
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Renderer.h"
#include "VeryCoolEngine/Renderer/RenderCommand.h"
#include "VeryCoolEngine/Renderer/Camera.h"
#include "VeryCoolEngine/Renderer/Mesh.h"



namespace VeryCoolEngine {

	class ImGuiLayer;

	struct Scene {
		Camera* camera;

	};

	class VCE_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		Window& GetWindow() const { return *_window; };

		Mesh* GenerateHeightmap(uint32_t x, uint32_t y);


		static Application* GetInstance() { return _spInstance; }

		Window* _window;

		bool mainThreadReady = false;
		bool renderThreadReady = false;
		bool renderThreadShouldRun = true;
		std::mutex sceneMutex;
		Scene scene;

		Renderer* _pRenderer;
		Camera _Camera;

		Shader* _pFullscreenShader;

		Mesh* _pMesh;
		Mesh* _pHeightmap;
		Texture2D* _pDebugTexture;
		TextureCube* _pCubemap;
	private:
		std::thread _renderThread;
		bool _running = true;
		bool OnWindowClose(WindowCloseEvent& e);
		LayerStack _layerStack;
		ImGuiLayer* _pImGuiLayer;
		static Application* _spInstance;

		

		

		//VertexArray* _pVertArray;
		//VertexBuffer* _pVertBuffer;
		//IndexBuffer* _pIndexBuffer;

		std::chrono::high_resolution_clock::time_point _LastFrameTime = std::chrono::high_resolution_clock::now();
		float _DeltaTime;
	};
	Application* CreateApplication();

}
