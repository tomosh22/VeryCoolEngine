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
	public:
		Camera* camera = nullptr;

		Shader* skyboxShader = nullptr;
		TextureCube* skybox = nullptr;

		std::vector<Mesh*> meshes{};
		std::vector<Renderer::Light> lights{};
		unsigned int numLights = 0;

		bool ready = false;

		void Reset() {
			ready = false;
			meshes.clear();
			meshes = std::vector<Mesh*>();
			lights.clear();
			lights.resize(100);
			numLights = 0;
		};

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

		


		static Application* GetInstance() { return _spInstance; }

		Window* _window;

		bool mainThreadReady = false;
		bool renderThreadReady = false;
		bool renderThreadShouldRun = true;
		bool renderInitialised = false;
		std::mutex sceneMutex;
		Scene* scene;

		Renderer* _pRenderer;
		Camera _Camera;

		Shader* _pFullscreenShader;

		Mesh* _pMesh;
		Mesh* _pHeightmap;
		Texture2D* _pDebugTexture;
		TextureCube* _pCubemap;
		ImGuiLayer* _pImGuiLayer;
		LayerStack _layerStack;
		bool _running = true;
		bool OnWindowClose(WindowCloseEvent& e);

	private:
		std::thread _renderThread;
		
		
		
		
		static Application* _spInstance;

		

		

		//VertexArray* _pVertArray;
		//VertexBuffer* _pVertBuffer;
		//IndexBuffer* _pIndexBuffer;

		std::chrono::high_resolution_clock::time_point _LastFrameTime = std::chrono::high_resolution_clock::now();
		float _DeltaTime;
	};
	Application* CreateApplication();

}
