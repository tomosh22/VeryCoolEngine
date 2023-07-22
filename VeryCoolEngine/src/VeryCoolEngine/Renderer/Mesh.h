#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Shader.h"
namespace VeryCoolEngine {

	class Material;//todo implement
	class Mesh
	{
	public:
		virtual ~Mesh() = default;

		//virtual void Bind() const = 0;
		//virtual void Unbind() const = 0;

		virtual void SetVertexArray(VertexArray* vertexArray) = 0;

		VertexArray* GetVertexArray() const { return _pVertexArray; }

		void SetShader(Shader* shader) { _pShader = shader; }
		Shader* GetShader() const { return _pShader; }

		static Mesh* Create();
	protected:
		VertexArray* _pVertexArray;
		Material* _pMaterial;
		Shader* _pShader;//#todo this should be in material
	};

}
