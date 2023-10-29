#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
#include "VeryCoolEngine/Transform.h"
#include "VeryCoolEngine/Renderer/PipelineSpecification.h"
namespace VeryCoolEngine {
	enum class MeshTopolgy {
		Triangles,
		TriangleStrips,
	};
	struct InstanceData {
	public:
		InstanceData(ShaderDataType type, unsigned int divisor, void* data, unsigned int numElements) : m_eType(type),m_uDivisor(divisor),m_pData(data), m_uNumElements(numElements) {};
		ShaderDataType m_eType;
		unsigned int m_uDivisor;
		void* m_pData;
		unsigned int m_uNumElements;
	};
	class Material;//todo implement
	class Mesh
	{
	public:
		virtual ~Mesh() {
			delete[] m_pxVertexPositions;
			delete[] m_pxUVs;
			delete[] m_pxNormals;
			delete[] m_pxTangents;
			delete[] m_pxBitangents;
		};

		//virtual void Bind() const = 0;
		//virtual void Unbind() const = 0;

		virtual void SetVertexArray(VertexArray* vertexArray) = 0;
		virtual VertexBuffer* CreateInstancedVertexBuffer() = 0;

		VertexArray* GetVertexArray() const { return m_pxVertexArray; }

		void SetShader(Shader* shader) { m_pxShader = shader; }
		Shader* GetShader() const { return m_pxShader; }

		void SetTexture(Texture2D* texture) { m_pxTexture = texture; }
		void SetBumpMap(Texture2D* bumpMap) { m_pxBumpMap = bumpMap; }
		void SetRoughnessTex(Texture2D* roughnessTex) { m_pxRoughnessTex = roughnessTex; }
		void SetMetallicTex(Texture2D* metallicTex) { m_pxMetallicTex = metallicTex; }
		void SetHeightmapTex(Texture2D* heightmapTex) { m_pxHeightmapTex = heightmapTex; }
		Texture2D* GetTexture() const { return m_pxTexture; }
		Texture2D* GetBumpMap() const { return m_pxBumpMap; }
		Texture2D* GetRoughnessTex() const { return m_pxRoughnessTex; }
		Texture2D* GetMetallicTex() const { return m_pxMetallicTex; }
		Texture2D* GetHeightmapTex() const { return m_pxHeightmapTex; }

		virtual void PlatformInit() = 0;

		static Mesh* Create();
		static Mesh* GenerateGenericHeightmap(uint32_t width, uint32_t height);
		static Mesh* GenerateQuad();
		static Mesh* GenerateVulkanTest();

		static Mesh* FromFile(const std::string& path, bool swapYZ = false);

		Transform m_xTransform;


		std::vector<BufferElement> m_axInstanceData;
		uint32_t m_uNumInstances = 1;

		uint32_t m_uNumVerts;
		uint32_t m_uNumIndices;

		BufferLayout* m_pxBufferLayout;

		MeshTopolgy m_eTopolgy = MeshTopolgy::Triangles;

		TextureDescriptorSpecification m_xTexDescSpec;

	protected:
		VertexArray* m_pxVertexArray;
		Material* m_pxMaterial;
		Shader* m_pxShader;//#todo this should be in material
		Texture2D* m_pxTexture = nullptr; //#todo so should this
		Texture2D* m_pxBumpMap = nullptr; //#todo so should this
		Texture2D* m_pxRoughnessTex = nullptr; //#todo so should this
		Texture2D* m_pxMetallicTex = nullptr; //#todo so should this
		Texture2D* m_pxHeightmapTex = nullptr; //#todo so should this

		glm::vec3* m_pxVertexPositions = nullptr;
		glm::vec2* m_pxUVs = nullptr;
		glm::vec3* m_pxNormals = nullptr;
		glm::vec3* m_pxTangents = nullptr;
		glm::vec3* m_pxBitangents = nullptr;
		unsigned int* m_puIndices = nullptr;
		
		void* m_pVerts;

		

		void GenerateNormals();
		void GenerateTangents();
		glm::vec4 GenerateTangent(uint32_t uA, uint32_t uB, uint32_t uC);

	};

}
