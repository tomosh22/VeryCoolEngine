#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>



namespace VeryCoolEngine {

	Renderer* Renderer::_spRenderer = nullptr;
	Renderer* RenderCommand::_spRenderer = Renderer::_spRenderer;

	Application* Application::_spInstance = nullptr;

	

	Application::Application() {
		srand(time(0));
		_spInstance = this;
#ifdef VCE_OPENGL
		_window = Window::Create();
		std::function callback = [this](Event&& e) {OnEvent(e); };
		_window->SetEventCallback(callback);
		_window->SetVSync(true);

		//_window->SetVSync(true);
#endif
		m_pxExampleSkinnedMesh = Mesh::FromFile("ogre.fbx");
		m_pxExampleMesh = Mesh::FromFile("cubeFlat.obj");
		SetupPipelines();
		
		
		_renderThread = std::thread([&]() {
			while (true) {
				printf("implement mutex\n");
				if (_renderThreadCanStart)break;//#todo implement mutex here
			}
#ifdef VCE_OPENGL
			_pRenderer->InitWindow();
#endif
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
		
		
		scene = new Scene();
		
		bool a = false;
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
			//#todo can be changed
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

		BufferDescriptorSpecification xCamSpec;
		xCamSpec.m_aeUniformBufferStages.push_back({ &_pCameraUBO, ShaderStageVertexAndFragment });

		BufferDescriptorSpecification xLightSpec;
		xLightSpec.m_aeUniformBufferStages.push_back({ &_pLightUBO, ShaderStageVertexAndFragment });

		TextureDescriptorSpecification xBlockTexSpec;
		xBlockTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });

		TextureDescriptorSpecification xFramebufferTexSpec;
		xFramebufferTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xFramebufferTexSpec.m_bJustFragment = true;
		xFramebufferTexSpec.m_bBindless = false;

		TextureDescriptorSpecification xMeshTexSpec;
		//currently overriding stage to all
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });

		m_pxMeshShader = Shader::Create("vulkan/meshVert.spv", "vulkan/meshFrag.spv", "", "vulkan/meshTesc.spv", "vulkan/meshTese.spv");
		m_pxGBufferShader = Shader::Create("vulkan/meshVert.spv", "vulkan/meshGBufferFrag.spv", "", "vulkan/meshTesc.spv", "vulkan/meshTese.spv");
		m_pxCopyToFramebufferShader = Shader::Create("vulkan/copyToFrameBufferVert.spv", "vulkan/copyToFrameBufferFrag.spv");
		m_pxSkinnedMeshShader = Shader::Create("vulkan/skinnedMeshVert.spv", "vulkan/meshFrag.spv");

		m_pxQuadMesh = Mesh::GenerateQuad();
		m_pxQuadMesh->SetShader(Shader::Create("vulkan/fullscreenVert.spv", "vulkan/fullscreenFrag.spv"));
		_meshes.push_back(m_pxQuadMesh);

		


		m_xPipelineSpecs.insert(
			{ "Skybox",
					PipelineSpecification(
					"Skybox",
					m_pxQuadMesh,
					m_pxQuadMesh->GetShader(),
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					true,
					false,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					{},
					{},
					&m_pxRenderToTexturePass,
					false,
					false,
					true,
					1,
					0
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
					{},
					{},
					&m_pxRenderToTexturePass,
					true,
					true,
					true,
					3,
					5
					)
			});

		/*m_xPipelineSpecs.insert(
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
					{},
					{},
					&m_pxRenderToTexturePass,
					true,
					false,
					true,
					4,
					5
					)
			});*/

		m_xPipelineSpecs.insert(
			{ "CopyToFramebuffer",
					PipelineSpecification(
					"CopyToFramebuffer",
					m_pxQuadMesh,
					m_pxCopyToFramebufferShader,
					{BlendFactor::SrcAlpha},
					{BlendFactor::OneMinusSrcAlpha},
					{true},
					false,
					false,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					{},
					{xFramebufferTexSpec},
					&m_pxCopyToFramebufferPass,
					false,
					false,
					true,
					0,
					1
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


	void Application::Run() {
		while (true) { 
			Sleep(1);
			if (renderInitialised)break;//#todo implement mutex here
		}
		while (_running) {
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			mainThreadReady = true;
			std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _LastFrameTime);
			_DeltaTime = duration.count()/1000.;
			_LastFrameTime = now;


			_Camera.UpdateCamera(_DeltaTime);
			for (Mesh* pxMesh : m_apxGenericMeshes) {
				pxMesh->m_xTransform.UpdateRotation();
				pxMesh->m_xTransform.UpdateMatrix();
			}
			GameLoop();
			

			
			
			for (Layer* layer : _layerStack)
				layer->OnUpdate();

			

			

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		}
		_renderThread.join();
	}
}