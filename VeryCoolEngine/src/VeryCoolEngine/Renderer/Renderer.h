#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"
#include "Mesh.h"
#include <glm/mat4x4.hpp>
#include "Shader.h"
#include "Camera.h"
#include "GraphicsContext.h"
#include "VeryCoolEngine/Renderer/ManagedUniformBuffer.h"

namespace VeryCoolEngine {
	class Renderer
	{
	public:
		static uint32_t _sMAXLIGHTS;
		struct Light {
			glm::vec3 position;
			float radius;
			glm::vec3 color;
		};

		virtual void InitWindow() = 0;
		virtual void PlatformInit()=0;
		void GenericInit();


		virtual void SetClearColor(const glm::vec4 color) = 0;
		virtual void Clear() = 0;
		
		virtual void BeginScene(glm::mat4 viewProjMat) = 0;
		virtual void EndScene() = 0;

		virtual void BindViewProjMat(Shader* shader) = 0;
		virtual void BindLightUBO(Shader* shader) = 0;

		virtual void DrawFullScreenQuad() = 0;

		virtual void RenderThreadFunction() = 0;

		static void Submit(VertexArray* vertexArray);
		static void SubmitMesh(Mesh* mesh);
		static void SubmitSkybox(Shader* shader, Camera* camera, TextureCube* cubemap);

		virtual void DrawIndexed(VertexArray* vertexArray) = 0;
		static Renderer* Create();

		static Renderer* _spRenderer;
		static GraphicsContext* _spContext;
		ManagedUniformBuffer* _pLightUBO = nullptr;
	private:
	};

	class RendererAPI {

	};
}


