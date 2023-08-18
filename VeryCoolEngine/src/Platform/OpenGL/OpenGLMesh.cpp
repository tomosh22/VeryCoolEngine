#include "vcepch.h"
#include "OpenGLMesh.h"

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
	if (instanceData != nullptr) {
		layout.GetElements().push_back({ instanceDataType, instanceDataName });
		//#todo do properly
		numFloats += 2;
		//numFloats += ShaderDataTypeSize(instanceDataType) / sizeof(float);
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
		if (instanceData != nullptr) {
			
			//#todo do this properly
			glm::ivec2 val = *((glm::ivec2*)instanceData);
			((int*)verts)[index++] = val.x;
			((int*)verts)[index++] = val.y;
			
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

	vertexArray->Unbind();


	SetVertexArray(vertexArray);
}
