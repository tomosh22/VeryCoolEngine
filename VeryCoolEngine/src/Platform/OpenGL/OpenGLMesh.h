#pragma once
#include "VeryCoolEngine/Renderer/Mesh.h"

namespace VeryCoolEngine {
	class OpenGLMesh : public Mesh
	{
	public:
		~OpenGLMesh() override {};

		void PlatformInit() override;
		//void Bind() const override;
		//void Unbind() const override;

		VertexBuffer* CreateInstancedVertexBuffer();

		void SetVertexArray(VertexArray* vertexArray) override { _pVertexArray = vertexArray; }

		std::vector<unsigned int> _instanceVBOs;//#todo make this part of vertex array
	};
}


