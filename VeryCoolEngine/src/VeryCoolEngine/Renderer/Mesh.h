#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
#include "VeryCoolEngine/Transform.h"
namespace VeryCoolEngine {
	enum class MeshTopolgy {
		Triangles,
		TriangleStrips
	};
	struct InstanceData {
	public:
		InstanceData(ShaderDataType type, unsigned int divisor, void* data, unsigned int numElements) : _type(type),_divisor(divisor),_data(data), _numElements(numElements) {};
		ShaderDataType _type;
		unsigned int _divisor;
		void* _data;
		unsigned int _numElements;
	};
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
		static Mesh* GenerateCubeFace();

		Transform transform;

		//#todo how do i want to handle this properly
		glm::ivec2 customUniform;

		std::vector<InstanceData> _instanceData;

	protected:
		VertexArray* _pVertexArray;
		Material* _pMaterial;
		Shader* _pShader;//#todo this should be in material
		Texture2D* _pTexture; //#todo so should this
		Texture2D* _pBumpMap; //#todo so should this

		uint32_t numVerts;
		uint32_t numIndices;
		glm::vec3* vertexPositions = nullptr;
		glm::vec2* uvs = nullptr;
		glm::vec3* normals = nullptr;
		glm::vec4* tangents = nullptr;
		unsigned int* indices = nullptr;

		


		void* verts;

		

		void GenerateNormals();
		void GenerateTangents();
		glm::vec4 GenerateTangent(uint32_t a, uint32_t b, uint32_t c);

	};

}
