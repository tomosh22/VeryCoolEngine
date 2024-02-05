#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include "Physics/Physics.h"
#include "GLFW/glfw3.h"



namespace VeryCoolEngine {

	Renderer* Renderer::_spRenderer = nullptr;
	Renderer* RenderCommand::_spRenderer = Renderer::_spRenderer;

	Application* Application::_spInstance = nullptr;

	

	Application::Application() {
		srand(time(0));
		_spInstance = this;

		m_pxFoliageModel = new VCEModel();
		m_pxFoliageModel->m_pxTransform = new reactphysics3d::Transform;
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

		m_apxModels.push_back(m_pxFoliageModel);

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

		
		
		scene = new Scene();

		m_xEditorCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
	}

	
	

	void Application::OnEvent(Event& e) {
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_ESCAPE) _running = false;
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_Q) _mouseEnabled = !_mouseEnabled;

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
		delete scene;
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
		m_apxModels.push_back(m_pxQuadModel);

		


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

	void Application::ConstructScene(float fDt)
	{
		sceneMutex.lock();
		scene->Reset();

		Physics::UpdatePhysics();

		scene->camera = m_eCurrentState == VCE_GAMESTATE_EDITOR ? &m_xEditorCamera : &m_xGameCamera;

		scene->skybox = _pCubemap;

		for (RendererAPI::Light& light : _lights) {
			scene->lights[scene->numLights++] = light;
		}

		scene->m_axPipelineMeshes.insert({ "Skybox", std::vector<VCEModel*>() });
		scene->m_axPipelineMeshes.at("Skybox").push_back(m_pxQuadModel);


		scene->m_axPipelineMeshes.insert({ "Meshes", std::vector<VCEModel*>() });

		scene->m_axPipelineMeshes.insert({ "SkinnedMeshes", std::vector<VCEModel*>() });
		for (VCEModel* pxModel : m_apxModels) {
			if (pxModel->m_pxAnimation != nullptr) {
				//has an animation
				pxModel->m_pxAnimation->UpdateAnimation(fDt / 1000.f);
				std::vector<glm::mat4>& xAnimMats = pxModel->m_pxAnimation->GetFinalBoneMatrices();
				for (Mesh* pxMesh : pxModel->m_apxMeshes) {
					for (uint32_t i = 0; i < pxMesh->m_xBoneMats.size(); i++) {
						pxMesh->m_xBoneMats.at(i) = xAnimMats.at(i);
					}
				}
				scene->m_axPipelineMeshes.at("SkinnedMeshes").push_back(pxModel);
			}
			else {
				//TODO: check this properly
				if (pxModel == m_pxQuadModel || pxModel == m_pxFoliageModel) continue;
				//does not have an animation
				//hacky way to make sure this mesh belongs in this pipeline
				if (pxModel->m_apxMeshes.back()->m_pxMaterial != nullptr)
					scene->m_axPipelineMeshes.at("Meshes").push_back(pxModel);

			}
		}

		scene->m_axPipelineMeshes.insert({ "GBuffer", std::vector<VCEModel*>() });
		for (VCEModel* model : m_apxModels)
			scene->m_axPipelineMeshes.at("GBuffer").push_back(model);

		for (RendererAPI::Light& light : _lights) {
			scene->lights[scene->numLights++] = light;
		}

		RendererAPI::Light camLight{
				m_xEditorCamera.GetPosition().x,m_xEditorCamera.GetPosition().y,m_xEditorCamera.GetPosition().z,100,
				1,1,1,1
		};
		scene->lights[scene->numLights++] = camLight;

		scene->ready = true;
		sceneMutex.unlock();
	}


	void Application::Run() {
		while (true) { 
			Sleep(1);
			if (renderInitialised)break;//#todo implement mutex here
		}

		ResetScene();

		while (_running) {
			mainThreadReady = true;

			std::chrono::high_resolution_clock::time_point fCurrentTime = std::chrono::high_resolution_clock::now();

			m_fDeltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(fCurrentTime - m_fLastFrameTime).count() / 1.e9;
			m_fLastFrameTime = fCurrentTime;

			if (m_eCurrentState != m_ePrevState) {
				sceneMutex.lock();
				Physics::ResetPhysics();
				ResetScene();
				
				switch (m_eCurrentState) {
				case VCE_GAMESTATE_PLAYING:
					break;
				case VCE_GAMESTATE_EDITOR:
					break;
				}
				sceneMutex.unlock();
				m_bSkipFrame = true;
			}
			m_ePrevState = m_eCurrentState;

			if (m_bSkipFrame) {
				m_bSkipFrame = false;
				continue;
			}

			if (m_eCurrentState == VCE_GAMESTATE_PLAYING)
				Physics::s_fTimestepAccumulator += m_fDeltaTime;

			if(m_eCurrentState == VCE_GAMESTATE_EDITOR)
				m_xEditorCamera.UpdateCamera(m_fDeltaTime);


			switch (m_eCurrentState) {
			case VCE_GAMESTATE_EDITOR:
				if (m_xEditorCamera.IsCursorInRendererViewport() && Input::IsMouseButtonPressed(VCE_MOUSE_BUTTON_LEFT)) {
					reactphysics3d::Ray xCursorRay = Physics::BuildRayFromMouse(&m_xEditorCamera);

					VCEModel* pxHitModel = nullptr;
					float fHitDistance = FLT_MAX;
					for (VCEModel* pxModel : m_apxModels) {
						if (!pxModel->m_bUsePhysics) continue;

						reactphysics3d::RaycastInfo xRayCastInfo;
						if (pxModel->m_pxRigidBody->raycast(xCursorRay, xRayCastInfo)) {
							float fNewDistance = reactphysics3d::Vector3(xRayCastInfo.worldPoint - xCursorRay.point1).length();
							if (fNewDistance < fHitDistance) {
								pxHitModel = pxModel;
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
				break;
			case VCE_GAMESTATE_PLAYING:
				GameLoop(m_fDeltaTime);
				m_pxSelectedModel = nullptr;
				break;
				VCE_ASSERT(false, "Invalid game state");
			}

			for (VCEModel* pxModel : m_apxModels)
				if(pxModel->m_bUsePhysics)
					pxModel->m_pxTransform = (reactphysics3d::Transform*)&pxModel->m_pxRigidBody->getTransform();
			
			ConstructScene(m_fDeltaTime);

			
			
			for (Layer* layer : _layerStack)
				layer->OnUpdate();


		}
		_renderThread.join();
	}
}