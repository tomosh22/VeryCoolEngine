#include "vcepch.h"
#include "OpenGLMesh.h"
#include <glad/glad.h>
#include "VeryCoolEngine/Application.h"

void VeryCoolEngine::OpenGLMesh::PlatformInit()
{

	int numFloats = 0;
	BufferLayout layout;
	if (vertexPositions != nullptr) {
		layout.GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
		numFloats += 3;
	}
	if (uvs != nullptr) {
		layout.GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
		numFloats += 2;
	}
	if (normals != nullptr) {
		layout.GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
		numFloats += 3;
	}
	if (tangents != nullptr) {
		layout.GetElements().push_back({ ShaderDataType::Float4, "_aTangent" });
		numFloats += 4;
	}

	verts = new float[numVerts * numFloats];

	size_t index = 0;
	for (int i = 0; i < numVerts; i++)
	{
		if (vertexPositions != nullptr) {
			((float*)verts)[index++] = vertexPositions[i].x;
			((float*)verts)[index++] = vertexPositions[i].y;
			((float*)verts)[index++] = vertexPositions[i].z;
		}

		if (uvs != nullptr) {
			((float*)verts)[index++] = uvs[i].x;
			((float*)verts)[index++] = uvs[i].y;
		}
		if (normals != nullptr) {
			((float*)verts)[index++] = normals[i].x;
			((float*)verts)[index++] = normals[i].y;
			((float*)verts)[index++] = normals[i].z;
		}
		if (tangents != nullptr) {
			((float*)verts)[index++] = tangents[i].x;
			((float*)verts)[index++] = tangents[i].y;
			((float*)verts)[index++] = tangents[i].z;
			((float*)verts)[index++] = tangents[i].w;
		}
}
	
#if 0
	BufferLayout layout = {
		{ShaderDataType::Float3, "_aPosition"},
		{ShaderDataType::Float2, "_aUV"},
		{ShaderDataType::Float3, "_aNormal"},
		{ShaderDataType::Float4, "_aTangent"}
	};
#else
	
	layout.CalculateOffsetsAndStrides();
#endif
	VertexBuffer* vertexBuffer = VertexBuffer::Create(verts, numVerts * sizeof(float) * (numFloats));
	vertexBuffer->SetLayout(layout);
	VertexArray* vertexArray = VertexArray::Create();
	vertexArray->AddVertexBuffer(vertexBuffer);

	IndexBuffer* indexBuffer = IndexBuffer::Create(indices, numIndices);
	vertexArray->SetIndexBuffer(indexBuffer);

	//#todo do properly
#pragma region DoProperly

	Application* app = Application::GetInstance();

	unsigned int mats;
	glGenBuffers(1, &mats);
	glBindBuffer(GL_ARRAY_BUFFER, mats);
	glBufferData(GL_ARRAY_BUFFER, app->_instanceMats.size() * sizeof(glm::mat4), app->_instanceMats.data(), GL_STATIC_DRAW);


	vertexArray->Bind();
	// vertex attributes
	std::size_t vec4Size = sizeof(glm::vec4);

	glEnableVertexAttribArray(vertexArray->_numVertAttribs);
	glVertexAttribPointer(vertexArray->_numVertAttribs, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glVertexAttribDivisor(vertexArray->_numVertAttribs++, 1);

	glEnableVertexAttribArray(vertexArray->_numVertAttribs);
	glVertexAttribPointer(vertexArray->_numVertAttribs, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
	glVertexAttribDivisor(vertexArray->_numVertAttribs++, 1);

	glEnableVertexAttribArray(vertexArray->_numVertAttribs);
	glVertexAttribPointer(vertexArray->_numVertAttribs, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glVertexAttribDivisor(vertexArray->_numVertAttribs++, 1);

	glEnableVertexAttribArray(vertexArray->_numVertAttribs);
	glVertexAttribPointer(vertexArray->_numVertAttribs, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
	glVertexAttribDivisor(vertexArray->_numVertAttribs++, 1);

	
	

	unsigned int offsets;
	glGenBuffers(1, &offsets);
	glBindBuffer(GL_ARRAY_BUFFER, offsets);
	glBufferData(GL_ARRAY_BUFFER, app->_instanceOffsets.size() * sizeof(glm::ivec2), app->_instanceOffsets.data(), GL_STATIC_DRAW);

	vertexArray->Bind();

	std::size_t ivec2Size = sizeof(glm::ivec2);
	glEnableVertexAttribArray(vertexArray->_numVertAttribs);
	glVertexAttribIPointer(vertexArray->_numVertAttribs, 2, GL_INT, ivec2Size, (void*)0);
	glVertexAttribDivisor(vertexArray->_numVertAttribs++, 6);

#pragma endregion


	vertexArray->Unbind();


	SetVertexArray(vertexArray);
}
