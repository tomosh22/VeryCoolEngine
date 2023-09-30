#pragma once
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "VeryCoolEngine/Renderer/Mesh.h"
#include "VeryCoolEngine/Renderer/RenderPass.h"

namespace VeryCoolEngine {
	class ManagedUniformBuffer;
	class Pipeline
	{
	public:
		Shader* m_pxShader;
		BufferLayout*  m_xVertexBufferLayout;
		MeshTopolgy m_xTopology; //TODO typo lol
		std::vector<ManagedUniformBuffer**> m_apxUBOs;
		RenderPass** m_pxRenderPass;
		Pipeline() = default;

		virtual void PlatformInit() = 0;
		static Pipeline* Create(Shader* pxShader, BufferLayout* xLayout, MeshTopolgy xTopology, std::vector<ManagedUniformBuffer**> apxUBOs, RenderPass** xRenderPass);

	};
}
