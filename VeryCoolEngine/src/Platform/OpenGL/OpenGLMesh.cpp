#include "vcepch.h"
#include "OpenGLMesh.h"
#include <glad/glad.h>
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {


	void OpenGLMesh::PlatformInit()
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



		if (_instanceData.size() > 0) {
			VertexBuffer* instancedVertexBuffer = CreateInstancedVertexBuffer();
			vertexArray->AddVertexBuffer(instancedVertexBuffer, true);
		}

		vertexArray->Unbind();


		SetVertexArray(vertexArray);
	}


	VertexBuffer* OpenGLMesh::CreateInstancedVertexBuffer()
	{
		BufferLayout instancedLayout;
		unsigned int instanceDataSize = 0;

		for (BufferElement& instanceData : _instanceData) {
			BufferElement element = BufferElement(instanceData._Type,
				instanceData._Name,
				false, //normalised
				true,  //instanced
				instanceData._divisor,
				instanceData._data,
				instanceData._numEntries);
			instancedLayout.GetElements().push_back(element);
			instanceDataSize += instanceData._Size * instanceData._numEntries;
		}


#ifdef VCE_DEBUG
		unsigned int iNumEntriesCheck = -1;
		unsigned int iNumElements = 0;
		for (BufferElement& instanceData : _instanceData) {
			if (iNumEntriesCheck == -1)iNumEntriesCheck = instanceData._numEntries;
			else VCE_ASSERT(instanceData._numEntries == iNumEntriesCheck, "don't support varying number of entries")
				VCE_ASSERT(instanceData._divisor == 1, "need to implement support for different divisors");
			iNumElements++;
		}
#else
		unsigned int iNumElements = _instanceData.size();
#endif

		char* pData = new char[instanceDataSize];

		//keeps track of how far into the overall buffer we are combining each element
		unsigned int iDataOffset = 0;

		//keeps track of how far into each individual element's data we are
		unsigned int* elementOffsets = new unsigned int[iNumElements] {0};


#ifdef VCE_DEBUG
		unsigned int testCount = 0;
#endif

		//interweaving instance data into pData
		//ex. position rotation colour position rotation colour
		//as opposed to position position rotation rotation colour colour

		while (iDataOffset < instanceDataSize) {
			unsigned int iCurrentElement = 0;
			for (BufferElement& instanceData : _instanceData) {
				int offset = elementOffsets[iCurrentElement];
				memcpy(pData + iDataOffset,
					(char*)instanceData._data + offset,
					instanceData._Size
				);
				elementOffsets[iCurrentElement] += instanceData._Size;
				iDataOffset += instanceData._Size;
				iCurrentElement++;
			}
#ifdef VCE_DEBUG
			testCount++;
#endif
		}

#ifdef VCE_DEBUG
		int testIndex = 0;
		for (BufferElement& instanceData : _instanceData) {
			VCE_ASSERT(elementOffsets[testIndex] == instanceData._Size * instanceData._numEntries, "error");
			testIndex++;
		}
		VCE_ASSERT(testCount == iNumEntriesCheck, "incorrect number of entries");
		VCE_ASSERT(iDataOffset == instanceDataSize, "incorrect data offset");
#endif

		delete[] elementOffsets;


		instancedLayout.CalculateOffsetsAndStrides();
		VertexBuffer* instancedVertexBuffer = VertexBuffer::Create(pData, instanceDataSize);
		instancedVertexBuffer->SetLayout(instancedLayout);
		

		delete[] pData;
		return instancedVertexBuffer;
	}
}