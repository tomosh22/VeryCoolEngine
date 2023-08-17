#include "vcepch.h"
#include "OpenGLMesh.h"

void VeryCoolEngine::OpenGLMesh::PlatformInit()
{
	
#if 0
	BufferLayout layout = {
		{ShaderDataType::Float3, "_aPosition"},
		{ShaderDataType::Float2, "_aUV"},
		{ShaderDataType::Float3, "_aNormal"},
		{ShaderDataType::Float4, "_aTangent"}
	};
#else
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
