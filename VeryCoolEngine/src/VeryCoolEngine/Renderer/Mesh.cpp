#include "vcepch.h"
#include "Mesh.h"
#include "Platform/Vulkan/VulkanMesh.h"

#include <unordered_set>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "VeryCoolEngine/AssetHandling/Assets.h"

namespace VeryCoolEngine {

	Mesh* Mesh::Create() {
#ifdef VCE_OPENGL
		return new OpenGLMesh();
#endif
#ifdef VCE_VULKAN
		return new VulkanMesh();
#endif
	}
	//credit rich davison
	//#todo cleanup, was written on laptop
	Mesh* Mesh::GenerateGenericHeightmap(uint32_t width, uint32_t height)
	{
		Mesh* mesh = Mesh::Create();
		mesh->m_pxBufferLayout = new BufferLayout();
		glm::vec3 vertexScale = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec2 textureScale = glm::vec2(100,100);
		mesh->m_uNumVerts = width * height;
		mesh->m_uNumIndices = (width - 1) * (height - 1) * 6;
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxUVs = new glm::vec2[mesh->m_uNumVerts];
		mesh->m_pxNormals = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxTangents = new glm::vec3[mesh->m_uNumVerts];
		for (size_t i = 0; i < mesh->m_uNumVerts; i++)
		{
			mesh->m_pxNormals[i] = { 0,0,0 };
			mesh->m_pxTangents[i] = { 0,0,0 };
		}
		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices];

		

		for (int z = 0; z < height; ++z) {
			for (int x = 0; x < width; ++x) {
				int offset = (z * width) + x;
				mesh->m_pxVertexPositions[offset] = glm::vec3(x, 100 + rand() % 1/*#todo read height tex*/, z) * vertexScale;
				glm::vec2 fUV = glm::vec2(x, z) / textureScale;
				mesh->m_pxUVs[offset] = fUV;
			}
		}

		size_t i = 0;
		for (int z = 0; z < height - 1; ++z) {
			for (int x = 0; x < width - 1; ++x) {
				int a = (z * width) + x;
				int b = (z * width) + x + 1;
				int c = ((z + 1) * width) + x + 1;
				int d = ((z + 1) * width) + x;
				mesh->m_puIndices[i++] = a;
				mesh->m_puIndices[i++] = c;
				mesh->m_puIndices[i++] = b;
				mesh->m_puIndices[i++] = c;
				mesh->m_puIndices[i++] = a;
				mesh->m_puIndices[i++] = d;
			}
		}
		
		mesh->GenerateNormals();
		mesh->GenerateTangents();

		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();

		return mesh;
	}
	
	Mesh* Mesh::GenerateQuad() {
		Mesh* mesh = Mesh::Create();
		mesh->m_pxBufferLayout = new BufferLayout();
		mesh->m_uNumVerts = 4;
		mesh->m_uNumIndices = 6;
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxUVs = new glm::vec2[mesh->m_uNumVerts];

		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices] {0, 2, 1, 2, 3, 1};

		mesh->m_pxVertexPositions[0] = { 0.5,0.5,1 };
		mesh->m_pxVertexPositions[1] = { 0.5,-0.5,1 };
		mesh->m_pxVertexPositions[2] = { -0.5,0.5,1 };
		mesh->m_pxVertexPositions[3] = { -0.5,-0.5,1 };

		mesh->m_pxUVs[0] = { 1,0 };
		mesh->m_pxUVs[1] = { 1,1 };
		mesh->m_pxUVs[2] = { 0,0 };
		mesh->m_pxUVs[3] = { 0,1 };

		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();

		return mesh;
	}


	Mesh* Mesh::GenerateVulkanTest()
	{
		Mesh* mesh = Mesh::Create();
		mesh->m_pxBufferLayout = new BufferLayout();
		mesh->m_uNumVerts = 4;
		mesh->m_uNumIndices = 6;
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxNormals = new glm::vec3[mesh->m_uNumVerts];

		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices] {0, 2, 1,1,2,3};

		/*mesh->vertexPositions[0] = { -0.5,-0.5,1 };
		mesh->vertexPositions[1] = { 0.5,-0.5,1 };
		mesh->vertexPositions[2] = { -0.5,0.5,1 };
		mesh->vertexPositions[3] = { 0.5,0.5,1 };*/

		mesh->m_pxVertexPositions[0] = { -50,-50,1 };
		mesh->m_pxVertexPositions[1] = { 50,-50,1 };
		mesh->m_pxVertexPositions[2] = { -50,50,1 };
		mesh->m_pxVertexPositions[3] = { 50,50,1 };

		mesh->m_pxNormals[0] = { 1,0,0 };
		mesh->m_pxNormals[1] = { 0,1,0 };
		mesh->m_pxNormals[2] = { 0,0,1 };
		mesh->m_pxNormals[3] = { 1,1,1 };

#pragma region MoveToGenericInitFunction
		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();
#pragma endregion
		return mesh;
	}
	

	//struct CompareVec3 final
	//{
	//	size_t operator()(const glm::vec3& k)const
	//	{
	//		return std::hash<int>()(k.x) ^ std::hash<int>()(k.y) ^ std::hash<int>()(k.z);
	//	}
	//
	//	bool operator()(const glm::vec3& a, const glm::vec3& b)const
	//	{
	//		return a.x == b.x && a.y == b.y && a.z == b.z;
	//	}
	//};

	

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		bool operator==(const Vertex& other) const {
			return pos == other.pos && uv == other.uv && normal == other.normal;
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
	

	Mesh* Mesh::FromFile(const std::string& path, bool swapYZ /* = false*/)
	{

		std::string strPath(MESHDIR);
		strPath += path;

		Assimp::Importer importer;
		const aiScene* pxScene = importer.ReadFile(strPath.c_str(),
			aiProcess_CalcTangentSpace |
			aiProcess_GenSmoothNormals |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		aiMesh* pxMesh = pxScene->mMeshes[0];

		std::vector<Vertex> axVertices;
		std::vector<uint32_t> auIndices;

		for (uint32_t i = 0; i < pxMesh->mNumVertices; i++) {
			const aiVector3D* pxPos = &(pxMesh->mVertices[i]);
			const aiVector3D* pxNormal = &(pxMesh->mNormals[i]);
			const aiVector3D* pxTexCoord = &(pxMesh->mTextureCoords[0][i]);
			const aiVector3D* pxTangent = &(pxMesh->mTangents[i]);
			const aiVector3D* pxBitangent = &(pxMesh->mBitangents[i]);

			Vertex v(
				glm::vec3(pxPos->x, pxPos->y, pxPos->z),
				glm::vec2(pxTexCoord->x, pxTexCoord->y),
				glm::vec3(pxNormal->x, pxNormal->y, pxNormal->z),
				glm::vec3(pxTangent->x, pxTangent->y, pxTangent->z),
				glm::vec3(pxBitangent->x, pxBitangent->y, pxBitangent->z)
			);

			axVertices.push_back(v);
		}

		for (uint32_t i = 0; i < pxMesh->mNumFaces; i++) {
			VCE_ASSERT(pxMesh->mFaces[i].mNumIndices == 3, "Face isn't a triangle");
			auIndices.push_back(pxMesh->mFaces[i].mIndices[0]);
			auIndices.push_back(pxMesh->mFaces[i].mIndices[1]);
			auIndices.push_back(pxMesh->mFaces[i].mIndices[2]);
		}


		Mesh* mesh = Mesh::Create();
		mesh->m_pxBufferLayout = new BufferLayout();

		mesh->m_uNumVerts = axVertices.size();
		mesh->m_uNumIndices = auIndices.size();

		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices];
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxNormals = new glm::vec3[mesh->m_uNumVerts]{ glm::vec3(0,0,0) };
		mesh->m_pxTangents = new glm::vec3[mesh->m_uNumVerts]{ glm::vec3(0,0,0) };
		mesh->m_pxBitangents = new glm::vec3[mesh->m_uNumVerts]{ glm::vec3(0,0,0) };
		mesh->m_pxUVs = new glm::vec2[mesh->m_uNumVerts];


		for (size_t i = 0; i < auIndices.size(); i++)mesh->m_puIndices[i] = auIndices[i];
		for (size_t i = 0; i < axVertices.size(); i++) {
			mesh->m_pxVertexPositions[i] = axVertices[i].pos;
			mesh->m_pxUVs[i] = axVertices[i].uv;
			mesh->m_pxNormals[i] = axVertices[i].normal;
			mesh->m_pxTangents[i] = axVertices[i].tangent;
			mesh->m_pxBitangents[i] = axVertices[i].bitangent;
		}


		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}
		if (mesh->m_pxBitangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aBitangent" });
			numFloats += 3;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
			if (mesh->m_pxBitangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBitangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBitangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBitangents[i].z;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();

		return mesh;

		return nullptr;
	}

	void Mesh::GenerateNormals()
	{
		for (size_t i = 0; i < m_uNumIndices / 3; i++)
		{
			int a = m_puIndices[i * 3];
			int b = m_puIndices[i * 3 + 1];
			int c = m_puIndices[i * 3 + 2];

			glm::vec3 posA = m_pxVertexPositions[a];// * glm::vec3(10,10,10);
			glm::vec3 posB = m_pxVertexPositions[b];// * glm::vec3(10, 10, 10);
			glm::vec3 posC = m_pxVertexPositions[c];// * glm::vec3(10, 10, 10);

			glm::vec3 normal = glm::cross(posB - posA, posC - posA);
			m_pxNormals[a] += normal;
			m_pxNormals[b] += normal;
			m_pxNormals[c] += normal;
		}

		for (size_t i = 0; i < m_uNumVerts; i++)
		{
			m_pxNormals[i] = glm::normalize(m_pxNormals[i]);
		}
	}
	void Mesh::GenerateTangents()
	{
#if 0
		for (uint32_t i = 0; i < m_uNumIndices/3; i++)
		{
			unsigned int a = m_puIndices[i * 3];
			unsigned int b = m_puIndices[i * 3 + 1];
			unsigned int c = m_puIndices[i * 3 + 2];
			glm::vec4 tangent = GenerateTangent(a, b, c);
			m_pxTangents[a] += tangent;
			m_pxTangents[b] += tangent;
			m_pxTangents[c] += tangent;
		}
		for (uint32_t i = 0; i < m_uNumVerts; i++)
		{
			float handedness = m_pxTangents[i].w > 0 ? 1.f : -1.f;
			m_pxTangents[i].w = 0;
			m_pxTangents[i] = glm::normalize(m_pxTangents[i]);
			m_pxTangents[i].w = handedness;
		}
#endif
	}
	glm::vec4 Mesh::GenerateTangent(uint32_t a, uint32_t b, uint32_t c) {
		glm::vec3 ba = m_pxVertexPositions[b] - m_pxVertexPositions[a];
		glm::vec3 ca = m_pxVertexPositions[c] - m_pxVertexPositions[a];
		glm::vec2 tba = m_pxUVs[b] - m_pxUVs[a];
		glm::vec2 tca = m_pxUVs[c] - m_pxUVs[a];

		glm::mat2 texMatrix(tba, tca);
		texMatrix = glm::inverse(texMatrix);

		glm::vec3 tangent = ba * texMatrix[0][0] + ca * texMatrix[0][1];
		glm::vec3 binormal = ba * texMatrix[1][0] + ca * texMatrix[1][1];

		glm::vec3 normal = glm::cross(ba,ca);
		glm::vec3 biCross = glm::cross(tangent,normal);

		float handedness = glm::dot(biCross,binormal) < 0.f ? -1.f : 1.f;
		return glm::vec4(tangent.x,tangent.y,tangent.z,handedness);
	}
}