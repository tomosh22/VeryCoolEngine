#include "vcepch.h"
#include "Mesh.h"
#include "Platform/OpenGL/OpenGLMesh.h"

namespace VeryCoolEngine {

	Mesh* Mesh::Create() {
#ifdef VCE_OPENGL
		return new OpenGLMesh();
#endif
	}
	//credit rich davison
	//#todo cleanup, was written on laptop
	Mesh* Mesh::GenerateHeightmap(uint32_t width, uint32_t height)
	{
		Mesh* mesh = Mesh::Create();
		glm::vec3 vertexScale = glm::vec3(8.0f, 1.0f, 8.0f);
		glm::vec2 textureScale = glm::vec2(1 / 2.0f, 1 / 2.0f);
		mesh->numVerts = width * height;
		mesh->numIndices = (width - 1) * (height - 1) * 6;
		mesh->vertices = new glm::vec3[mesh->numVerts];
		mesh->uvs = new glm::vec2[mesh->numVerts];
		mesh->normals = new glm::vec3[mesh->numVerts];
		mesh->tangents = new glm::vec4[mesh->numVerts];
		for (size_t i = 0; i < mesh->numVerts; i++)
		{
			mesh->normals[i] = { 0,0,0 };
			mesh->tangents[i] = { 0,0,0,0 };
		}
		mesh->indices = new unsigned int[mesh->numIndices];

		VertexArray* vertexArray = VertexArray::Create();

		for (int z = 0; z < height; ++z) {
			for (int x = 0; x < width; ++x) {
				int offset = (z * width) + x;
				mesh->vertices[offset] = glm::vec3(x, rand() % 50 /*#todo read height tex*/, z) * vertexScale;
				mesh->uvs[offset] = glm::vec2(x, z) * textureScale;
			}
		}

		size_t i = 0;
		for (int z = 0; z < height - 1; ++z) {
			for (int x = 0; x < width - 1; ++x) {
				int a = (z * width) + x;
				int b = (z * width) + x + 1;
				int c = ((z + 1) * width) + x + 1;
				int d = ((z + 1) * width) + x;
				mesh->indices[i++] = a;
				mesh->indices[i++] = c;
				mesh->indices[i++] = b;
				mesh->indices[i++] = c;
				mesh->indices[i++] = a;
				mesh->indices[i++] = d;
			}
		}
		
		mesh->GenerateNormals();
		mesh->GenerateTangents();

		float* verts = new float[mesh->numVerts * (3 + 2 + 3 + 4)];
		size_t index = 0;
		for (i = 0; i < mesh->numVerts; i++)
		{
			verts[index++] = mesh->vertices[i].x;
			verts[index++] = mesh->vertices[i].y;
			verts[index++] = mesh->vertices[i].z;

			verts[index++] = mesh->uvs[i].x;
			verts[index++] = mesh->uvs[i].y;

			verts[index++] = mesh->normals[i].x;
			verts[index++] = mesh->normals[i].y;
			verts[index++] = mesh->normals[i].z;

			verts[index++] = mesh->tangents[i].x;
			verts[index++] = mesh->tangents[i].y;
			verts[index++] = mesh->tangents[i].z;
			verts[index++] = mesh->tangents[i].w;
		}

		VertexBuffer* vertexBuffer = VertexBuffer::Create(verts, mesh->numVerts * sizeof(float) * (3 + 2 + 3 + 4));

		BufferLayout layout = {
			{ShaderDataType::Float3, "_aPosition"},
			{ShaderDataType::Float2, "_aUV"},
			{ShaderDataType::Float3, "_aNormal"},
			{ShaderDataType::Float4, "_aTangent"}
		};
		vertexBuffer->SetLayout(layout);

		vertexArray->AddVertexBuffer(vertexBuffer);

		IndexBuffer* indexBuffer = IndexBuffer::Create(mesh->indices, mesh->numIndices);
		vertexArray->SetIndexBuffer(indexBuffer);

		vertexArray->Unbind();

		
		mesh->SetVertexArray(vertexArray);
		return mesh;
	}
	void Mesh::GenerateNormals()
	{
		for (size_t i = 0; i < numIndices / 3; i++)
		{
			int a = indices[i * 3];
			int b = indices[i * 3 + 1];
			int c = indices[i * 3 + 2];

			glm::vec3 normal = glm::cross(vertices[b] - vertices[a], vertices[c] - vertices[a]);
			normals[a] += normal;
			normals[b] += normal;
			normals[c] += normal;
		}

		for (size_t i = 0; i < numVerts; i++)
		{
			normals[i] = glm::normalize(normals[i]);
		}
	}
	void Mesh::GenerateTangents()
	{
		for (uint32_t i = 0; i < numIndices/3; i++)
		{
			unsigned int a = indices[i * 3];
			unsigned int b = indices[i * 3 + 1];
			unsigned int c = indices[i * 3 + 2];
			glm::vec4 tangent = GenerateTangent(a, b, c);
			tangents[a] += tangent;
			tangents[b] += tangent;
			tangents[c] += tangent;
		}
		for (uint32_t i = 0; i < numVerts; i++)
		{
			float handedness = tangents[i].w > 0 ? 1.f : -1.f;
			tangents[i].w = 0;
			tangents[i] = glm::normalize(tangents[i]);
			tangents[i].w = handedness;
		}
	}
	glm::vec4 Mesh::GenerateTangent(uint32_t a, uint32_t b, uint32_t c) {
		glm::vec3 ba = vertices[b] - vertices[a];
		glm::vec3 ca = vertices[c] - vertices[a];
		glm::vec2 tba = uvs[b] - uvs[a];
		glm::vec2 tca = uvs[c] - uvs[a];

		glm::mat2 texMatrix(tba, tca);

		glm::vec3 tangent = ba * texMatrix[0][0] + ca * texMatrix[0][1];
		glm::vec3 binormal = ba * texMatrix[1][0] + ca * texMatrix[1][1];

		glm::vec3 normal = glm::cross(ba,ca);
		glm::vec3 biCross = glm::cross(tangent,normal);

		float handedness = glm::dot(biCross,binormal) < 0.f ? -1.f : 1.f;
		return glm::vec4(tangent.x,tangent.y,tangent.z,handedness);
	}
}