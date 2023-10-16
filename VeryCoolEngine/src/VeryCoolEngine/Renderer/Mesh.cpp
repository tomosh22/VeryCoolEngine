#include "vcepch.h"
#include "Mesh.h"
#include "Platform/Vulkan/VulkanMesh.h"

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
		mesh->m_pxTangents = new glm::vec4[mesh->m_uNumVerts];
		for (size_t i = 0; i < mesh->m_uNumVerts; i++)
		{
			mesh->m_pxNormals[i] = { 0,0,0 };
			mesh->m_pxTangents[i] = { 0,0,0,0 };
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
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float4, "_aTangent" });
			numFloats += 4;
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
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].w;
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
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float4, "_aTangent" });
			numFloats += 4;
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
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].w;
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
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float4, "_aTangent" });
			numFloats += 4;
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
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].w;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();
#pragma endregion
		return mesh;
	}

	void Mesh::GenerateNormals()
	{
		for (size_t i = 0; i < m_uNumIndices / 3; i++)
		{
			int a = m_puIndices[i * 3];
			int b = m_puIndices[i * 3 + 1];
			int c = m_puIndices[i * 3 + 2];

			glm::vec3 normal = glm::cross(m_pxVertexPositions[b] - m_pxVertexPositions[a], m_pxVertexPositions[c] - m_pxVertexPositions[a]);
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
	}
	glm::vec4 Mesh::GenerateTangent(uint32_t a, uint32_t b, uint32_t c) {
		glm::vec3 ba = m_pxVertexPositions[b] - m_pxVertexPositions[a];
		glm::vec3 ca = m_pxVertexPositions[c] - m_pxVertexPositions[a];
		glm::vec2 tba = m_pxUVs[b] - m_pxUVs[a];
		glm::vec2 tca = m_pxUVs[c] - m_pxUVs[a];

		glm::mat2 texMatrix(tba, tca);

		glm::vec3 tangent = ba * texMatrix[0][0] + ca * texMatrix[0][1];
		glm::vec3 binormal = ba * texMatrix[1][0] + ca * texMatrix[1][1];

		glm::vec3 normal = glm::cross(ba,ca);
		glm::vec3 biCross = glm::cross(tangent,normal);

		float handedness = glm::dot(biCross,binormal) < 0.f ? -1.f : 1.f;
		return glm::vec4(tangent.x,tangent.y,tangent.z,handedness);
	}
}