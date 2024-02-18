#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"
#include "Mesh.h"
#include <glm/mat4x4.hpp>
#include "Shader.h"
#include "Camera.h"
#include "GraphicsContext.h"
#include "VeryCoolEngine/Renderer/ManagedUniformBuffer.h"
#include "RendererAPI.h"

namespace VeryCoolEngine {
	
	class RendererScene;
	class Renderer
	{
	public:

#ifdef VCE_USE_EDITOR
		int m_width = VCE_GAME_WIDTH + VCE_EDITOR_ADDITIONAL_WIDTH, m_height = VCE_GAME_HEIGHT + VCE_EDITOR_ADDITIONAL_HEIGHT;
#else
		int m_width = VCE_GAME_WIDTH, m_height = VCE_GAME_HEIGHT;
#endif

		virtual void MainLoop() = 0;

		virtual void InitWindow() = 0;
		virtual void PlatformInit()=0;
		void GenericInit();

		virtual void InitialiseAssets() = 0;
		virtual void CleanupAssets() = 0;

		virtual void WaitDeviceIdle() = 0;

		virtual void OnResize(uint32_t uWidth, uint32_t uHeight) = 0;


		virtual void SetClearColor(const glm::vec4 color) = 0;
		virtual void Clear() = 0;
		
		virtual void BeginScene(RendererScene* scene) = 0;
		virtual void EndScene() = 0;

		virtual void BindViewProjMat(Shader* shader) = 0;
		virtual void BindLightUBO(Shader* shader) = 0;

		virtual void DrawFullScreenQuad() = 0;

		virtual void RenderThreadFunction() = 0;

		static void Submit(VertexArray* vertexArray);
		static void SubmitMesh(Mesh* mesh);
		static void SubmitMeshInstanced(Mesh* mesh, unsigned int count);
		static void SubmitSkybox(Shader* shader, Camera* camera, TextureCube* cubemap);

		virtual void DrawIndexed(VertexArray* vertexArray, MeshTopolgy topology = MeshTopolgy::Triangles) = 0;
		virtual void DrawIndexedInstanced(VertexArray* vertexArray, unsigned int count, MeshTopolgy topology = MeshTopolgy::Triangles) = 0;
		static Renderer* Create();

		static Renderer* _spRenderer;
		static GraphicsContext* _spContext;
		

		bool m_bShouldResize = false;
		glm::vec3 m_xOverrideNormal = { 0,0,0 };
		bool m_bUseBumpMaps = true;
		bool m_bUsePhongTess = true;
		float m_fPhongTessFactor = 1.0f;
		uint32_t m_uTessLevel = 1;
		bool bAnimate = true;
		float fAnimAlpha = 1;
		bool m_bDisableDrawCalls = false;

		virtual void ProfilingBeginFrame() = 0;
		virtual void RecordDrawCall() = 0;
		virtual void ProfilingEndFrame() = 0;
		uint32_t m_uNumDrawCalls = 0;

		
	private:
	};

}


