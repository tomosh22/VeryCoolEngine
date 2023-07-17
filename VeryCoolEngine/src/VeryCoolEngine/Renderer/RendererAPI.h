#pragma once
#include <glm/glm.hpp>

namespace VeryCoolEngine {
	class RendererAPI
	{
	public:

		virtual void SetClearColor(const glm::vec4 color) = 0;
		virtual void Clear() = 0;
		virtual void DrawIndexed() = 0;


	};
}


