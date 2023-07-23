#include "vcepch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Input.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include "Platform/OpenGL/OpenGLShader.h"

namespace VeryCoolEngine {

	Renderer* Renderer::_spRenderer = Renderer::Create();
	Renderer* RenderCommand::_spRenderer = Renderer::_spRenderer;

	Application* Application::_spInstance = nullptr;

	

	Application::Application() {
		srand(time(0));
		_spInstance = this;
		_window = Window::Create();
		std::function callback = [this](Event& e) {OnEvent(e); };
		_window->SetEventCallback(callback);
		_window->SetVSync(true);

		_pRenderer = Renderer::_spRenderer;
		_pRenderer->Init();

		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 0, 5), 0, 0, 45, 0, 1000);
		//_Camera = Camera::BuildOrthoCamera(glm::vec3(0, 0, -5), 0, 0, -10, 10, 5, -5, 1, 100);

		_pImGuiLayer = new ImGuiLayer();
		PushOverlay(_pImGuiLayer);

		VertexArray* vertexArray = VertexArray::Create();


		float verts[3 * (3+4)] = {
			-5,-5,0,    1,0,0,1,
			5,-5,0,     0,1,0,1,
			0,5,0,        0,0,1,1
		};
		
		VertexBuffer* vertexBuffer = VertexBuffer::Create(verts,sizeof(verts));

		BufferLayout layout = {
			{ShaderDataType::Float3, "_aPosition"},
			{ShaderDataType::Float4, "_aColor"},
		};
		vertexBuffer->SetLayout(layout);
		
		vertexArray->AddVertexBuffer(vertexBuffer);
		
		unsigned int indices[3] = { 0,1,2 };
		IndexBuffer* indexBuffer = IndexBuffer::Create(indices, 3);
		vertexArray->SetIndexBuffer(indexBuffer);

		vertexArray->Unbind();

		//_pMesh = Mesh::Create();
		//_pMesh->SetVertexArray(vertexArray);

		//_pMesh->SetShader( Shader::Create("basic.vert", "basic.frag"));

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		_pMesh = GenerateHeightmap(10, 10);
		_pMesh->SetShader(Shader::Create("basic.vert", "basic.frag"));
		_pMesh->SetTexture(Texture2D::Create("test1024x1024.png", false));

		_pFullscreenShader = Shader::Create("fullscreen.vert", "fullscreen.frag");
		bool a = false;
	}

	//credit rich davison
	//#todo cleanup, was written on laptop
	Mesh* Application::GenerateHeightmap(uint32_t width, uint32_t height) {
		glm::vec3 vertexScale = glm::vec3(16.0f, 1.0f, 16.0f);
		glm::vec2 textureScale = glm::vec2(1 / 8.0f, 1 / 8.0f);
		uint32_t numVerts = width * height;
		uint32_t numIndices = (width - 1) * (height - 1) * 6;
		glm::vec3* vertices = new glm::vec3[numVerts];
		glm::vec2* uvs = new glm::vec2[numVerts];
		GLuint* indices = new GLuint[numIndices];

		VertexArray* vertexArray = VertexArray::Create();

		for (int z = 0; z < height; ++z) {
			for (int x = 0; x < width; ++x) {
				int offset = (z * width) + x;
				vertices[offset] = glm::vec3(x, rand() % 10 /*#todo read height tex*/, z) * vertexScale;
				uvs[offset] = glm::vec2(x, z) * textureScale;
			}
		}

		size_t i = 0;
		for (int z = 0; z < height - 1; ++z) {
			for (int x = 0; x < width - 1; ++x) {
				int a = (z * (width)) + x;
				int b = (z * (width)) + (x + 1);
				int c = ((z + 1) * (width)) + (x + 1);
				int d = ((z + 1) * (width)) + x;

				indices[i++] = a;
				indices[i++] = c;
				indices[i++] = b;

				indices[i++] = c;
				indices[i++] = a;
				indices[i++] = d;
			}
		}

		float* verts = new float[numVerts * (3 + 2)];
		size_t index = 0;
		for (i = 0; i < numVerts; i++)
		{
			verts[index++] = vertices[i].x;
			verts[index++] = vertices[i].y;
			verts[index++] = vertices[i].z;

			verts[index++] = uvs[i].x;
			verts[index++] = uvs[i].y;
		}

		VertexBuffer* vertexBuffer = VertexBuffer::Create(verts, numVerts * sizeof(float) * (3+2));

		BufferLayout layout = {
			{ShaderDataType::Float3, "_aPosition"},
			{ShaderDataType::Float2, "_aUV"},
		};
		vertexBuffer->SetLayout(layout);

		vertexArray->AddVertexBuffer(vertexBuffer);

		IndexBuffer* indexBuffer = IndexBuffer::Create(indices, numIndices);
		vertexArray->SetIndexBuffer(indexBuffer);

		vertexArray->Unbind();

		Mesh* mesh = Mesh::Create();
		mesh->SetVertexArray(vertexArray);
		return mesh;
	}

	void Application::OnEvent(Event& e) {
		if (e.GetType() == EventType::KeyPressed && dynamic_cast<KeyPressedEvent&>(e).GetKeyCode() == VCE_KEY_ESCAPE) _running = false;
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
		//delete _pCamera;
	}

	void Application::Run() {
		while (_running) {

			std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _LastFrameTime);
			_DeltaTime = duration.count()/1000.;
			//std::cout << "delta time: " << _DeltaTime << std::endl;
			_LastFrameTime = now;

			RenderCommand::SetClearColor({ 0.6, 0.2, 0.4, 1 });
			RenderCommand::Clear();


			
			_Camera.UpdateCamera(_DeltaTime);
			glm::mat4 viewProjMat = _Camera.BuildProjectionMatrix() * _Camera.BuildViewMatrix();

			_pRenderer->BeginScene(viewProjMat);

			_pRenderer->DrawFullScreenQuad(_pFullscreenShader,&_Camera);

			_pRenderer->SubmitMesh(_pMesh);

			
			for (Layer* layer : _layerStack)
				layer->OnUpdate();

			_pImGuiLayer->Begin();
			for (Layer* layer : _layerStack)
				layer->OnImGuiRender();
			_pImGuiLayer->End();

			_window->OnUpdate();
		}
	}
}