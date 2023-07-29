#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
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

		void SetTexture(Texture* texture) { _pTexture = texture; }
		Texture* GetTexture() const { return _pTexture; }

		static Mesh* Create();

	protected:
		VertexArray* _pVertexArray;
		Material* _pMaterial;
		Shader* _pShader;//#todo this should be in material
		Texture* _pTexture; //#todo so should this

	};

}
