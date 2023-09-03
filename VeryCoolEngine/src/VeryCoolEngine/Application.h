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
		std::vector<std::function<void(void)>> _functionsToRun;
		Camera* camera = nullptr;

		Shader* skyboxShader = nullptr;
		TextureCube* skybox = nullptr;

		std::vector<Mesh*> meshes{};

		Mesh* _pInstancedMesh;
		unsigned int _numInstances = 0;

		std::vector<Mesh*> instanceMeshes{};
		std::vector<glm::vec2> instanceData{};

		std::vector<Renderer::Light> lights{};
		unsigned int numLights = 0;

		bool ready = false;

		void Reset() {
			ready = false;
			meshes.clear();
			meshes = std::vector<Mesh*>();
			instanceMeshes.clear();
			instanceMeshes = std::vector<Mesh*>();
			lights.clear();
			lights.resize(Renderer::_sMAXLIGHTS);
			numLights = 0;
			_numInstances = 0;
			_pInstancedMesh = nullptr;

		};

	};

	class VCE_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
		void GameLoop();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		Window& GetWindow() const { return *_window; };

		


		static Application* GetInstance() { return _spInstance; }

		Window* _window;

		bool _renderThreadCanStart = false;
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
		unsigned int _numInstances = 0;

		std::vector<Mesh*> _meshes;
		std::vector<Mesh*> _instanceMeshes;
		std::vector<Shader*> _shaders;
		std::vector<Texture2D*> _textures;

		Mesh* _pHeightmap;
		std::vector<Renderer::Light> _lights{};
		Texture2D* _pDebugTexture;
		TextureCube* _pCubemap;
		ImGuiLayer* _pImGuiLayer;
		LayerStack _layerStack;
		bool _running = true;
		bool OnWindowClose(WindowCloseEvent& e);
		bool _mouseEnabled = true;

		//#todo temporary, need to implement a proper uniform system
		bool _aoEnabled = true;

		bool prevRState = false;

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
