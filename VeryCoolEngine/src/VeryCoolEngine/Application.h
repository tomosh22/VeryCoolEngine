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
#include "VeryCoolEngine/Renderer/Model.h"
#include "VeryCoolEngine/Renderer/PipelineSpecification.h"
#include "VeryCoolEngine/Renderer/Animation.h"
#include "Physics/Physics.h"
#include "Scene/Entity.h"

//#define VCE_DEFERRED_SHADING


namespace VeryCoolEngine {

	class ImGuiLayer;

	class RendererScene {
	public:
		Camera* camera = nullptr;

		

		Shader* skyboxShader = nullptr;
		TextureCube* skybox = nullptr;

		inline void AddPipeline(const char* szName) {
			m_axPipelineMeshes.insert({ szName, std::vector<VCEModel*>() });
		}
		inline void AddModelToPipeline(const char* szName, VCEModel* pxModel) {
			m_axPipelineMeshes.at(szName).push_back(pxModel);
		}

		const std::vector<VCEModel*>& GetModelsInPipeline(const char* szName) const {
			return m_axPipelineMeshes.at(szName);
		}

		std::vector<RendererAPI::Light> lights{};
		unsigned int numLights = 0;

		bool ready = false;

		void Reset() {
			ready = false;
			lights.clear();
			lights.resize(RendererAPI::g_uMaxLights);
			m_axPipelineMeshes.clear();
			m_axPipelineMeshes = std::unordered_map<std::string, std::vector<VCEModel*>>();
			numLights = 0;

		};
	private:
		std::unordered_map<std::string, std::vector<VCEModel*>> m_axPipelineMeshes;
	};

	class VCE_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
		void GameLoop(float fDt);
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		Window& GetWindow() const { return *_window; };

		void SetupPipelines();

		static Application* GetInstance() { return _spInstance; }

		void ConstructRendererScene(float fDt);

		void CollisionCallback(VCEModel* pxModel1, VCEModel* pxModel2, Physics::CollisionEventType eType);

		//don't like that I can't use uint32_t, blame imgui
		enum GameState : int {
			VCE_GAMESTATE_EDITOR,
			VCE_GAMESTATE_PLAYING
		};
		int m_eCurrentState = VCE_GAMESTATE_EDITOR;
		int m_ePrevState = m_eCurrentState;
		bool m_bSkipFrame = false;

		void OnApplicationBegin();

		void ResetScene();

#pragma region ShouldBeInGame
		VCEModel* m_pxPlayerModel;
		VCEModel* m_pxGroundPlane;
		bool m_bPlayerIsOnFloor = false;
		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Material* pxMaterial, Transform xTrans);

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Transform xTrans);
#pragma endregion

		Window* _window;

		bool _renderThreadCanStart = false;
		bool mainThreadReady = false;
		bool renderThreadReady = false;
		bool renderThreadShouldRun = true;
		bool renderInitialised = false;
		std::mutex sceneMutex;
		RendererScene* m_pxRendererScene;

		class Scene* m_pxCurrentScene = nullptr;

		Renderer* _pRenderer;
		//RenderPass* m_pxBackbufferRenderPass;
		RenderPass* m_pxImguiRenderPass;//imgui doesn't use depth buffer
		RenderPass* m_pxGBufferRenderPass;
		RenderPass* m_pxRenderToTexturePass;
		RenderPass* m_pxRenderToTexturePassNoClear;
		RenderPass* m_pxCopyToFramebufferPass;


		Texture2D* m_pxBlankTexture2D;

		VCEModel* m_pxFoliageModel;
		std::vector<glm::vec3> m_xTestFoliagePositions;
		FoliageMaterial* m_pxFoliageMaterial;

		
		std::unordered_map<std::string, PipelineSpecification> m_xPipelineSpecs;
		

		ManagedUniformBuffer* _pLightUBO = nullptr;
		ManagedUniformBuffer* _pCameraUBO = nullptr;

		//don't know where i want to put this yet
		struct MeshRenderData {
			glm::vec3 xOverrideNormal;
			int uUseBumpMap;
			int uUsePhongTess;
			float fPhongTessFactor;
			int uTessLevel;
		};
		ManagedUniformBuffer* m_pxMiscMeshRenderDataUBO = nullptr;

		class BlockWorld* m_pxBlockWorld = nullptr;
		std::vector<Shader*> _shaders;
		//std::vector<Texture*> _textures;

		//for fullscreen pass
		VCEModel* m_pxQuadModel;

		Mesh* m_pxInstanceMesh;

		//#TODO i really need a better way to do this, used to provide vertex input state to mesh pipeline
		Mesh* m_pxExampleMesh;
		Mesh* m_pxExampleSkinnedMesh;
		std::vector<VCEModel*> m_apxModels;

		std::unordered_map<std::string, Material*> m_xMaterialMap;

		Shader* m_pxMeshShader;
		Shader* m_pxSkinnedMeshShader;
		Shader* m_pxGBufferShader;
		Shader* m_pxCopyToFramebufferShader;
		Shader* m_pxFoliageShader;

		Mesh* _pHeightmap;
		std::vector<RendererAPI::Light> _lights{};
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

		double m_fDeltaTime;
		std::chrono::high_resolution_clock::time_point m_fLastFrameTime;

		VCEModel* m_pxSelectedModel = nullptr;

	private:
		std::thread _renderThread;
		
		
		
		static Application* _spInstance;
	};
	Application* CreateApplication();

}
