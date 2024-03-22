#pragma once
#include "VeryCoolEngine/Renderer/RendererAPI.h"

namespace VeryCoolEngine {
	class Scene;
	class Mesh;
	class FoliageRenderer
	{
	public:
		FoliageRenderer();
		~FoliageRenderer();
		void RenderFoliage(Scene* pxScene);
		RendererAPI::CommandBuffer* m_pxFoliageCommandBuffer;
		Mesh* m_pxQuadMesh;
	};

}