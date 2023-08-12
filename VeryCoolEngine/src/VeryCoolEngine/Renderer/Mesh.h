#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
namespace VeryCoolEngine {

	class Material;//todo implement
	class Mesh
	{
	public:
		virtual ~Mesh() { delete[] vertexPositions; };

		//virtual void Bind() const = 0;
		//virtual void Unbind() const = 0;

		virtual void SetVertexArray(VertexArray* vertexArray) = 0;

		VertexArray* GetVertexArray() const { return _pVertexArray; }

		void SetShader(Shader* shader) { _pShader = shader; }
		Shader* GetShader() const { return _pShader; }

		void SetTexture(Texture2D* texture) { _pTexture = texture; }
		void SetBumpMap(Texture2D* bumpMap) { _pBumpMap = bumpMap; }
		Texture2D* GetTexture() const { return _pTexture; }
		Texture2D* GetBumpMap() const { return _pBumpMap; }

		virtual void PlatformInit() = 0;

		static Mesh* Create();
		static Mesh* GenerateGenericHeightmap(uint32_t width, uint32_t height);
	protected:
		VertexArray* _pVertexArray;
		Material* _pMaterial;
		Shader* _pShader;//#todo this should be in material
		Texture2D* _pTexture; //#todo so should this
		Texture2D* _pBumpMap; //#todo so should this

		uint32_t numVerts;
		uint32_t numIndices;
		glm::vec3* vertexPositions;
		glm::vec2* uvs;
		glm::vec3* normals;
		glm::vec4* tangents;
		unsigned int* indices;

		float* verts;

		void GenerateNormals();
		void GenerateTangents();
		glm::vec4 GenerateTangent(uint32_t a, uint32_t b, uint32_t c);

	};

}
