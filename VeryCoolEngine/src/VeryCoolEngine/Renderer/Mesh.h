//credit learnopengl.com
#pragma once
#include "VeryCoolEngine/Renderer/VertexArray.h"
#include "VeryCoolEngine/Renderer/Buffer.h"
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VeryCoolEngine/Renderer/Texture.h"
#include "VeryCoolEngine/Renderer/Material.h"
#include "VeryCoolEngine/Transform.h"
#include "VeryCoolEngine/Renderer/PipelineSpecification.h"
#include <map>
#define MAX_BONES_PER_VERTEX 8

class aiNodeAnim;
typedef float ai_real;
template<typename TReal>
class aiMatrix4x4t;
typedef aiMatrix4x4t<ai_real> aiMatrix4x4;
struct aiMesh;
struct aiScene;

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

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		int m_BoneIDs[MAX_BONES_PER_VERTEX]{ 0 };
		float m_Weights[MAX_BONES_PER_VERTEX]{ 0.0f };
		uint32_t m_uNumBones = 0;

		bool operator==(const Vertex& other) const {
			return pos == other.pos && uv == other.uv && normal == other.normal

				//just to be safe
				&& m_BoneIDs[0] == other.m_BoneIDs[0];
		}
	};
	struct VertexHash
	{
		size_t operator()(Vertex const& vertex) const {
			size_t posHash = std::hash<int>()(vertex.pos.x) ^ std::hash<int>()(vertex.pos.y) ^ std::hash<int>()(vertex.pos.z);
			size_t uvHash = std::hash<int>()(vertex.uv.x) ^ std::hash<int>()(vertex.uv.y);
			size_t normalHash = std::hash<int>()(vertex.normal.x) ^ std::hash<int>()(vertex.normal.y) ^ std::hash<int>()(vertex.normal.z);
			return posHash ^ uvHash ^ normalHash;
		}
	};

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

		virtual void SetVertexArray(VertexArray* vertexArray) = 0;
		virtual VertexBuffer* CreateInstancedVertexBuffer() = 0;

		VertexArray* GetVertexArray() const { return m_pxVertexArray; }

		void SetShader(Shader* shader) { m_pxShader = shader; }
		Shader* GetShader() const { return m_pxShader; }

		static void ExtractBoneWeightForVertices(Mesh* pxMesh, std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

		friend class VCEModel;

#ifdef VCE_MATERIAL_TEXTURE_DESC_SET
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
#endif

		virtual void PlatformInit() = 0;

		static Mesh* Create();
		static Mesh* GenerateGenericHeightmap(uint32_t width, uint32_t height);
		static Mesh* GenerateQuad(float fScale = 1.0f);
		static Mesh* GenerateVulkanTest();

		static Mesh* FromFile(const std::string& path, bool swapYZ = false);


		std::vector<BufferElement> m_axInstanceData;
		uint32_t m_uNumInstances = 1;

		uint32_t m_uNumVerts;
		uint32_t m_uNumIndices;

		BufferLayout* m_pxBufferLayout;

		MeshTopolgy m_eTopolgy = MeshTopolgy::Triangles;

		


		std::vector<Vertex> m_axVertices;
		std::vector<uint32_t> m_auIndices;
		//std::vector<Texture*> m_apxTextures;

		static glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from);

		bool m_bInitialised = false;

		int m_BoneCounter = 0;
		struct BoneInfo
		{
			int id;
			glm::mat4 offset;
		};

		static void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
		{
			vertex.m_BoneIDs[vertex.m_uNumBones] = boneID;
			vertex.m_Weights[vertex.m_uNumBones] = weight;
			vertex.m_uNumBones++;
			return;
			for (int i = 0; i < MAX_BONES_PER_VERTEX; ++i)
			{
				if (vertex.m_BoneIDs[i] == 0)
				{
					vertex.m_Weights[i] = weight;
					vertex.m_BoneIDs[i] = boneID;
					break;
				}
			}
		}

		

		struct BoneData {
			uint32_t m_auIDs[MAX_BONES_PER_VERTEX]{ 0 };
			float m_afWeights[MAX_BONES_PER_VERTEX]{ 0 };
			void AddBoneData(uint32_t uBoneID, float fWeight) {
				for (uint32_t i = 0; i < MAX_BONES_PER_VERTEX; i++) {
					if (m_afWeights[i] == 0.f) {
						m_auIDs[i] = uBoneID;
						m_afWeights[i] = fWeight;
						return;
					}
				}
			}
		};

		std::map<std::string, aiNodeAnim*> m_xNodeNameToAnim;
		std::map<std::string, uint32_t> m_xBoneNameToIndex;
		std::map<uint32_t, std::string> m_xBoneIndexToName;
		std::vector<glm::mat4> m_xBoneMats;

		Material* m_pxMaterial = nullptr;

	protected:
		VertexArray* m_pxVertexArray;
		Shader* m_pxShader;//#todo this should be in material
		

		
		uint32_t m_uNumBones = 0;
		
		std::vector<BoneData> m_xBoneData;
		
		
		std::map<uint32_t, std::vector<std::pair<uint32_t, float>>> m_xBoneInfluences;

		glm::vec3* m_pxVertexPositions = nullptr;
		glm::vec2* m_pxUVs = nullptr;
		glm::vec3* m_pxNormals = nullptr;
		glm::vec3* m_pxTangents = nullptr;
		glm::vec3* m_pxBitangents = nullptr;
		unsigned int* m_puIndices = nullptr;
		BoneData* m_pxBoneDatas = nullptr;

		
		void* m_pVerts;

		

		void GenerateNormals();
		void GenerateTangents();
		glm::vec4 GenerateTangent(uint32_t uA, uint32_t uB, uint32_t uC);

	};

}
