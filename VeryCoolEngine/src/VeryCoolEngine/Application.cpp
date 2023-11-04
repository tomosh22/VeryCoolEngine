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

		TextureDescriptorSpecification xMeshTexSpec;
		//currently overriding stage to all
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });

		m_pxMeshShader = Shader::Create("vulkan/meshVert.spv", "vulkan/meshFrag.spv", "", "vulkan/meshTesc.spv", "vulkan/meshTese.spv");
		m_pxGBufferShader = Shader::Create("vulkan/meshVert.spv", "vulkan/meshGBufferFrag.spv", "", "vulkan/meshTesc.spv", "vulkan/meshTese.spv");

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
					false,
					false,
					DepthCompareFunc::GreaterOrEqual,
					{ColourFormat::BGRA8_sRGB},
					DepthFormat::D32_SFloat,
					{xCamSpec},
					{},
					&m_pxBackbufferRenderPass,
					false,
					false
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
					{xCamSpec, xLightSpec},
					{xMeshTexSpec},
					&m_pxBackbufferRenderPass,
					true,
					true
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
					true
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
			GameLoop();
			

			
			
			for (Layer* layer : _layerStack)
				layer->OnUpdate();

			

			

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		}
		_renderThread.join();
	}
}