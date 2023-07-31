#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
namespace VeryCoolEngine {

	class Material;//todo implement
	class Mesh
	{
	public:
		virtual ~Mesh() { delete[] vertices; };

		//virtual void Bind() const = 0;
		//virtual void Unbind() const = 0;

		virtual void SetVertexArray(VertexArray* vertexArray) = 0;

		VertexArray* GetVertexArray() const { return _pVertexArray; }

		void SetShader(Shader* shader) { _pShader = shader; }
		Shader* GetShader() const { return _pShader; }

		void SetTexture(Texture* texture) { _pTexture = texture; }
		void SetBumpMap(Texture* bumpMap) { _pBumpMap = bumpMap; }
		Texture* GetTexture() const { return _pTexture; }
		Texture* GetBumpMap() const { return _pBumpMap; }

		static Mesh* Create();
		static Mesh* GenerateHeightmap(uint32_t width, uint32_t height);
	protected:
		VertexArray* _pVertexArray;
		Material* _pMaterial;
		Shader* _pShader;//#todo this should be in material
		Texture* _pTexture; //#todo so should this
		Texture* _pBumpMap; //#todo so should this

		uint32_t numVerts;
		uint32_t numIndices;
		glm::vec3* vertices;
		glm::vec2* uvs;
		glm::vec3* normals;
		glm::vec4* tangents;
		unsigned int* indices;

		void GenerateNormals();
		void GenerateTangents();
		glm::vec4 GenerateTangent(uint32_t a, uint32_t b, uint32_t c);

	};

}
