#pragma once
#include "VeryCoolEngine/Renderer/Mesh.h"

namespace VeryCoolEngine {
	class OpenGLMesh : public Mesh
	{
		~OpenGLMesh() override {};

		//void Bind() const override;
		//void Unbind() const override;

		void SetVertexArray(VertexArray* vertexArray) override { _pVertexArray = vertexArray; }
	};
}


