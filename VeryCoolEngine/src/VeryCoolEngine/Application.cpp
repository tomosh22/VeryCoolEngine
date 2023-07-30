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
		//_window->SetVSync(true);
		
		//_window->SetVSync(true);
		_pRenderer = Renderer::Create();
		
		_renderThread = std::thread([&]() {
			_pRenderer->InitWindow();
			_pRenderer->RenderThreadFunction();
		});

		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 0, 5), 0, 0, 45, 1, 1000, 1280.f/720.f);
		//_Camera = Camera::BuildOrthoCamera(glm::vec3(0, 0, -5), 0, 0, -10, 10, 5, -5, 1, 100);

		


		//_pMesh = Mesh::Create();
		//_pMesh->SetVertexArray(vertexArray);

		//_pMesh->SetShader( Shader::Create("basic.vert", "basic.frag"));

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		scene = new Scene();
		
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
		glm::vec3* normals = new glm::vec3[numVerts];
		for (size_t i = 0; i < numVerts; i++)
		{
			normals[i] = { 0,0,0 };
		}
		GLuint* indices = new GLuint[numIndices];

		VertexArray* vertexArray = VertexArray::Create();

		for (int z = 0; z < height; ++z) {
			for (int x = 0; x < width; ++x) {
				int offset = (z * width) + x;
				vertices[offset] = glm::vec3(x, rand() % 50 /*#todo read height tex*/, z) * vertexScale;
				uvs[offset] = glm::vec2(x, z) * textureScale;
			}
		}

		size_t i = 0;
		for (int z = 0; z < height - 1; ++z) {
			for (int x = 0; x < width - 1; ++x) {
				int a = (z * width) + x;
				int b = (z * width) + x + 1;
				int c = ((z + 1) * width) + x + 1;
				int d = ((z + 1) * width) + x;
				indices[i++] = a;
				indices[i++] = c;
				indices[i++] = b;
				indices[i++] = c;
				indices[i++] = a;
				indices[i++] = d;
			}
		}
		for (size_t i = 0; i < numIndices/3; i++)
		{
		int a = indices[i * 3];
		int b = indices[i * 3 + 1];
		int c = indices[i * 3 + 2];

		glm::vec3 normal = glm::cross(vertices[b] - vertices[a], vertices[c] - vertices[a]);
		normals[a] += normal;
		normals[b] += normal;
		normals[c] += normal;
		}

		for (size_t i = 0; i < numVerts; i++)
		{
			normals[i] = glm::normalize(normals[i]);
		}

		float* verts = new float[numVerts * (3 + 2 + 3)];
		size_t index = 0;
		for (i = 0; i < numVerts; i++)
		{
			verts[index++] = vertices[i].x;
			verts[index++] = vertices[i].y;
			verts[index++] = vertices[i].z;

			verts[index++] = uvs[i].x;
			verts[index++] = uvs[i].y;

			verts[index++] = normals[i].x;
			verts[index++] = normals[i].y;
			verts[index++] = normals[i].z;
		}

		VertexBuffer* vertexBuffer = VertexBuffer::Create(verts, numVerts * sizeof(float) * (3+2+3));

		BufferLayout layout = {
			{ShaderDataType::Float3, "_aPosition"},
			{ShaderDataType::Float2, "_aUV"},
			{ShaderDataType::Float3, "_aNormal"},
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
		delete scene;
		//delete _pCamera;
	}

	

	



	void Application::Run() {
		while (!renderInitialised) {}
		while (_running) {
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			mainThreadReady = true;
			std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
			std::chrono::duration duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _LastFrameTime);
			_DeltaTime = duration.count()/1000.;
			//std::cout << "delta time: " << _DeltaTime << std::endl;
			_LastFrameTime = now;
			_Camera.UpdateCamera(_DeltaTime);
			//while (!renderThreadReady) {
				//std::cout << "waiting for render thread" << std::endl;
			//}
			sceneMutex.lock();
			scene->Reset();
			
			scene->camera = &_Camera;
			scene->skyboxShader = _pFullscreenShader;
			scene->skybox = _pCubemap;
			scene->meshes.push_back(_pMesh);
			scene->lights[scene->numLights++] = {
				1,2,3,4,
				1,0,0,1
				};
			scene->lights[scene->numLights++] = {
				1,2,3,4,
				0,1,0,1
			};
			scene->lights[scene->numLights++] = {
				1,2,3,4,
				0,0,1,1
			};
			scene->lights[scene->numLights++] = {
				1,2,3,4,
				1,0,1,1
			};
			scene->ready = true;
			sceneMutex.unlock();

			
			for (Layer* layer : _layerStack)
				layer->OnUpdate();

			

			

			std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
			std::chrono::duration frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			//std::cout << "main thread duration: " << std::to_string(frameDuration.count() / 1000.f) << "ms" << std::endl;
		}
		_renderThread.join();
	}
}