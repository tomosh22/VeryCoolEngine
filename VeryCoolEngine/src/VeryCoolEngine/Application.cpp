#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include "Physics/Physics.h"
#include "GLFW/glfw3.h"

#include "Components/ModelComponent.h"
#include "Components/ColliderComponent.h"
#include "Components/ScriptComponent.h"
#ifdef VCE_VULKAN
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#endif

namespace VeryCoolEngine {

	Renderer* Renderer::_spRenderer = nullptr;
	Renderer* RenderCommand::_spRenderer = Renderer::_spRenderer;

	Application* Application::_spInstance = nullptr;

	

	Application::Application() {
		srand(time(0));
		_spInstance = this;

		m_pxFoliageModel = new VCEModel();
		m_pxExampleSkinnedMesh = Mesh::FromFile("ogre.fbx");
		m_pxExampleMesh = Mesh::FromFile("cubeFlat.obj");
		m_pxFoliageModel->m_apxMeshes.emplace_back(Mesh::GenerateQuad(10));
		m_pxFoliageMaterial = FoliageMaterial::Create("foliage1k");
		m_pxFoliageModel->m_strDirectory = "FoliageModel";

		m_xTestFoliagePositions = {
			{40.1,70,0},
			{39.9,70.25,0.01},
			{40.1,70.5,0.02},
			{39.9,70.75,0.03},
			{40.1,71,0.04},
			{39.9,71.25,0.05},
			{40.1,71.5,0.06},
			{39.9,71.75,0.07}
		};


		m_pxFoliageModel->m_apxMeshes.back()->m_axInstanceData.push_back(BufferElement(
			ShaderDataType::Float3,
			"_aInstancePosition",
			false,
			true,
			1,
			m_xTestFoliagePositions.data(),
			m_xTestFoliagePositions.size()
		));

		m_pxFoliageModel->m_apxMeshes.back()->m_uNumInstances = m_xTestFoliagePositions.size();

		SetupPipelines();
		
		Physics::InitPhysics();
		
		_renderThread = std::thread([&]() {
			while (true) {
				std::this_thread::yield();
				if (_renderThreadCanStart)break;//#todo implement mutex here
			}

#ifdef VCE_VULKAN
			_window = Window::Create();
			std::function callback = [this](Event&& e) {OnEvent(e); };
			_window->SetEventCallback(callback);
			_window->SetVSync(true);
			_pRenderer = Renderer::Create();
			Renderer::_spRenderer = _pRenderer;
#endif
			_pRenderer->RenderThreadFunction();
		});

		
		
		m_pxBlankTexture2D = Texture2D::Create(1, 1, TextureFormat::RGBA);

		
		
		m_pxRendererScene = new RendererScene();
	}

	
	

	void Application::OnEvent(Event& e) {
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_ESCAPE) _running = false;
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_Q)
			_window->ToggleCaptureCursor();

		if (e.GetType() == EventType::KeyPressed) {
			Input::s_xPressedKeys.insert(dynamic_cast<KeyPressedEvent&>(e).GetKeyCode());
		}

		if (e.GetType() == EventType::WindowResize) {
			WindowResizeEvent& xWindowResizeEvent = (WindowResizeEvent&)e;
			_window->SetWidth(xWindowResizeEvent.GetWidth());
			_window->SetHeight(xWindowResizeEvent.GetHeight());
			_pRenderer->m_bShouldResize = true;
		}
		
		EventDispatcher dispatcher(e);
		if (e.GetType() == EventType::WindowClose) {
			std::function function = [&](WindowCloseEvent& e) -> bool {return Application::OnWindowClose(e); };
			dispatcher.Dispatch(function);
		}

		for (auto it = _layerStack.end(); it != _layerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.GetHandled()) break;
		}
	}

	void Application::PushLayer(Layer* layer) {
		_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer) {
		_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		_running = false;
		return true;
	}

	Application::~Application() { 
		delete _window;
		delete m_pxRendererScene;
	}

	

	
	void Application::SetupPipelines() {

		m_pxMeshShader = Shader::Create("vulkan/meshVert.spv", "vulkan/meshFrag.spv", "", "vulkan/meshTesc.spv", "vulkan/meshTese.spv");
		m_pxGBufferShader = Shader::Create("vulkan/meshVert.spv", "vulkan/meshGBufferFrag.spv", "", "vulkan/meshTesc.spv", "vulkan/meshTese.spv");
		m_pxCopyToFramebufferShader = Shader::Create("vulkan/copyToFrameBufferVert.spv", "vulkan/copyToFrameBufferFrag.spv");
		m_pxSkinnedMeshShader = Shader::Create("vulkan/skinnedMeshVert.spv", "vulkan/meshFrag.spv");
		m_pxFoliageShader = Shader::Create("vulkan/foliageVert.spv", "vulkan/foliageFrag.spv");

		m_pxQuadModel = new VCEModel();
		m_pxQuadModel->m_bShowInEditor = false;
		m_pxQuadModel->m_apxMeshes.emplace_back(Mesh::GenerateQuad());
		m_pxQuadModel->m_apxMeshes.back()->SetShader(Shader::Create("vulkan/fullscreenVert.spv", "vulkan/fullscreenFrag.spv"));
		m_pxQuadModel->m_strDirectory = "QuadModel";

		


		m_xPipelineSpecs.insert(
			{ "Skybox",
					PipelineSpecification(
					"Skybox",
					m_pxQuadModel->m_apxMeshes.back(),
					m_pxQuadModel->m_apxMeshes.back()->GetShader(),
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					true,
					false,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					"RenderToTextureClear",
					false,
					false,
					{{3,0}}
					)
			});

		
		

		m_xPipelineSpecs.insert(
			{ "Meshes",
					PipelineSpecification(
					"Meshes",
					m_pxExampleMesh,
					m_pxMeshShader,
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					"RenderToTextureNoClear",
					true,
					true,
					{
						{3,0},
						{0,5}
					}
					)
			});

		m_xPipelineSpecs.insert(
			{ "SkinnedMeshes",
					PipelineSpecification(
					"SkinnedMeshes",
					m_pxExampleSkinnedMesh,
					m_pxSkinnedMeshShader,
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					"RenderToTextureNoClear",
					true,
					false,
					{
						{3,0},
						{0,5},
						{1,0}
					}
					)
			});

		m_xPipelineSpecs.insert(
			{ "CopyToFramebuffer",
					PipelineSpecification(
					"CopyToFramebuffer",
					m_pxQuadModel->m_apxMeshes.back(),
					m_pxCopyToFramebufferShader,
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					false,
					false,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					"CopyToFramebuffer",
					false,
					false,
					{
						{0,1}
					}
					)
			});

		m_xPipelineSpecs.insert(
			{ "Foliage",
					PipelineSpecification(
					"Foliage",
					m_pxFoliageModel->m_apxMeshes.back(),
					m_pxFoliageShader,
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					"RenderToTextureNoClear",
					false,
					false,
					{
						{1,0},
						{0,6}
					}
					)
			});

		//#TODO: not ready for pipeline to be made yet
#ifdef VCE_DEFERRED_SHADING
		m_xPipelineSpecs.insert(
			{ "GBuffer",
					PipelineSpecification(
					"GBuffer",
					m_pxExampleMesh,
					m_pxGBufferShader,
					{BlendFactor::SrcAlpha, BlendFactor::SrcAlpha, BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::OneMinusSrcAlpha},
					{true, true, true},
					true,
					true,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_Unorm, ColourFormat::BGRA8_Unorm, ColourFormat::BGRA8_Unorm},
					DepthFormat::D32_SFloat,
					{xCamSpec, xLightSpec},
					{xMeshTexSpec},
					&m_pxGBufferRenderPass,
					true,
					true,
					false,
					0,
					0
					)
			});
#endif

	

	}



	void Application::ConstructRendererScene(float fDt)
	{
		sceneMutex.lock();
		m_pxRendererScene->Reset();

		Physics::UpdatePhysics();

		m_pxRendererScene->camera = m_eCurrentState == VCE_GAMESTATE_EDITOR ? &m_pxCurrentScene->m_xEditorCamera : &m_pxCurrentScene->m_xGameCamera;

		m_pxRendererScene->m_pxScene = m_pxCurrentScene;

		m_pxRendererScene->skybox = _pCubemap;

		for (RendererAPI::Light& light : _lights) {
			m_pxRendererScene->lights[m_pxRendererScene->numLights++] = light;
		}

		m_pxRendererScene->AddPipeline("Skybox");
		m_pxRendererScene->AddModelToPipeline("Skybox", m_pxQuadModel);


		m_pxRendererScene->AddPipeline("Meshes");
		m_pxRendererScene->AddPipeline("SkinnedMeshes");

#ifdef VCE_DEFERRED_SHADING
		m_pxRendererScene->AddPipeline("GBuffer");
		for (VCEModel* model : m_apxModels)
			m_pxRendererScene->AddModelToPipeline("GBuffer", model);
#endif

		for (RendererAPI::Light& light : _lights) {
			m_pxRendererScene->lights[m_pxRendererScene->numLights++] = light;
		}

		m_pxRendererScene->ready = true;
		sceneMutex.unlock();
	}

	void Application::UpdateDeltaTime() {
		std::chrono::high_resolution_clock::time_point fCurrentTime = std::chrono::high_resolution_clock::now();

		m_fDeltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(fCurrentTime - m_fLastFrameTime).count() / 1.e9;
		m_fLastFrameTime = fCurrentTime;
	}

	void Application::UpdateEditorState() {
		m_pxCurrentScene->m_xEditorCamera.UpdateCamera(m_fDeltaTime);
		if (m_pxCurrentScene->m_xEditorCamera.IsCursorInRendererViewport() && Input::IsMouseButtonPressed(VCE_MOUSE_BUTTON_LEFT)) {
			reactphysics3d::Ray xCursorRay = Physics::BuildRayFromMouse(&m_pxCurrentScene->m_xEditorCamera);

			VCEModel* pxHitModel = nullptr;
			float fHitDistance = FLT_MAX;

			std::vector<ColliderComponent*> xColliders = m_pxCurrentScene->GetAllColliderComponents();
			for (ColliderComponent* pxCollider : xColliders) {
				Entity& xEntity = pxCollider->m_xParentEntity;
				if (!xEntity.HasComponent<ModelComponent>()) continue;


				reactphysics3d::RaycastInfo xRayCastInfo;
				if (pxCollider->GetRigidBody()->raycast(xCursorRay, xRayCastInfo)) {
					float fNewDistance = reactphysics3d::Vector3(xRayCastInfo.worldPoint - xCursorRay.point1).length();
					if (fNewDistance < fHitDistance) {
						pxHitModel = xEntity.GetComponent<ModelComponent>().GetModel();
						fHitDistance = fNewDistance;
					}
				}
			}

			if (pxHitModel != nullptr)
				VCE_TRACE("Hit {}", pxHitModel->m_strDirectory);
			else
				VCE_TRACE("Hit nothing");

			m_pxSelectedModel = pxHitModel;
		}
	}

	void Application::UpdateGameState() {
		Physics::s_fTimestepAccumulator += m_fDeltaTime;
		for (ScriptComponent* pxScript : m_pxCurrentScene->GetAllOfComponentType<ScriptComponent>())
			pxScript->OnUpdate(m_fDeltaTime);
		GameLoop(m_fDeltaTime);
		m_pxSelectedModel = nullptr;
	}

	void Application::HandleStateChange() {
		sceneMutex.lock();

		m_pxCurrentScene->Reset();

		switch (m_eCurrentState) {
		case VCE_GAMESTATE_PLAYING:
			_window->EnableCaptureCursor();
			break;
		case VCE_GAMESTATE_EDITOR:
			_window->DisableCaptureCursor();
			break;
		}
		sceneMutex.unlock();
		m_bSkipFrame = true;
	}


	void Application::Run() {
		while (true) { 
			std::this_thread::yield();
			if (renderInitialised)break;//#todo implement mutex here
		}

#ifdef VCE_USE_EDITOR
		while (!m_bImGuiInitialised)
			std::this_thread::yield();
#endif
		m_xAssetHandler.LoadAssetsFromFile("Assets.vceassets");

		for(uint32_t i = 0; i < 15; i++)
			m_xAssetHandler.LoadAssetsFromFile(std::to_string(i) + ".vceassets");

		m_xAssetHandler.PlatformInitialiseAssets();
		m_pxCurrentScene = new Scene("TestScene.vcescene");

#ifdef VCE_USE_EDITOR
		_window->DisableCaptureCursor();
#else
		_window->EnableCaptureCursor();
#endif

		m_xAsyncLoaderThread = std::thread([&]() {
			m_xAsyncLoader.ThreadFunc();
		});
		

		while (_running) {

			UpdateDeltaTime();
			

			if (m_bWantToResetScene) {
				m_bSkipFrame = true;
				m_pxRendererScene->ready = false;
				_pRenderer->WaitDeviceIdle();
				sceneMutex.lock();
				_pRenderer->CleanupAssets();
				delete m_pxCurrentScene;
				m_pxCurrentScene = new Scene("TestScene.vcescene");
				m_bWantToResetScene = false;
				sceneMutex.unlock();
			}

			if (m_eCurrentState != m_ePrevState) {
				HandleStateChange();
			}
			m_ePrevState = m_eCurrentState;

			if (m_bSkipFrame) {
				m_bSkipFrame = false;
				continue;
			}

			switch (m_eCurrentState) {
			case VCE_GAMESTATE_EDITOR:
				UpdateEditorState();
				break;
			case VCE_GAMESTATE_PLAYING:
				UpdateGameState();
				break;
#ifdef VCE_DEBUG
			default:
				VCE_ASSERT(false, "Invalid game state");
				break;
#endif
			}

			ConstructRendererScene(m_fDeltaTime);

			for (Layer* layer : _layerStack)
				layer->OnUpdate();

			
		}
		_renderThread.join();
		m_xAsyncLoaderThread.join();
	}
}