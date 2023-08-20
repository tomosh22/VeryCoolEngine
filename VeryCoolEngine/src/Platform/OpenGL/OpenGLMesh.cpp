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

	//#todo make this part of vertex array
	for (BufferElement& instanceData : _instanceData) {

		vertexBuffer = VertexBuffer::Create(instanceData._data, instanceData._numEntries * ShaderDataTypeSize(instanceData._Type));

		BufferLayout layout = {
		BufferElement(
			instanceData._Type,
			instanceData._Name,
			false,
			true,
			instanceData._divisor,
			instanceData._data,
			instanceData._numEntries
		)
		};
		vertexBuffer->SetLayout(layout);
		vertexArray->AddVertexBuffer(vertexBuffer, true);
	}



	vertexArray->Unbind();


	SetVertexArray(vertexArray);
}
