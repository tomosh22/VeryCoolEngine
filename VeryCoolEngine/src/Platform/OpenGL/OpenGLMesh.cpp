#include "vcepch.h"
#include "OpenGLMesh.h"
#include <glad/glad.h>
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {


	void OpenGLMesh::PlatformInit()
	{

		int numFloats = 0;
		BufferLayout layout;
		if (m_pxVertexPositions != nullptr) {
			layout.GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (m_pxUVs != nullptr) {
			layout.GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (m_pxNormals != nullptr) {
			layout.GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (m_pxTangents != nullptr) {
			layout.GetElements().push_back({ ShaderDataType::Float4, "_aTangent" });
			numFloats += 4;
		}

		m_pVerts = new float[m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < m_uNumVerts; i++)
		{
			if (m_pxVertexPositions != nullptr) {
				((float*)m_pVerts)[index++] = m_pxVertexPositions[i].x;
				((float*)m_pVerts)[index++] = m_pxVertexPositions[i].y;
				((float*)m_pVerts)[index++] = m_pxVertexPositions[i].z;
			}

			if (m_pxUVs != nullptr) {
				((float*)m_pVerts)[index++] = m_pxUVs[i].x;
				((float*)m_pVerts)[index++] = m_pxUVs[i].y;
			}
			if (m_pxNormals != nullptr) {
				((float*)m_pVerts)[index++] = m_pxNormals[i].x;
				((float*)m_pVerts)[index++] = m_pxNormals[i].y;
				((float*)m_pVerts)[index++] = m_pxNormals[i].z;
			}
			if (m_pxTangents != nullptr) {
				((float*)m_pVerts)[index++] = m_pxTangents[i].x;
				((float*)m_pVerts)[index++] = m_pxTangents[i].y;
				((float*)m_pVerts)[index++] = m_pxTangents[i].z;
				((float*)m_pVerts)[index++] = m_pxTangents[i].w;
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
		VertexBuffer* vertexBuffer = VertexBuffer::Create(m_pVerts, m_uNumVerts * sizeof(float) * (numFloats));
		vertexBuffer->SetLayout(layout);
		VertexArray* vertexArray = VertexArray::Create();
		vertexArray->AddVertexBuffer(vertexBuffer);

		IndexBuffer* indexBuffer = IndexBuffer::Create(m_puIndices, m_uNumIndices);
		vertexArray->SetIndexBuffer(indexBuffer);



		if (m_axInstanceData.size() > 0) {
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

		for (BufferElement& instanceData : m_axInstanceData) {
			BufferElement element = BufferElement(instanceData._Type,
				instanceData._Name,
				false, //normalised
				true,  //instanced
				instanceData.m_uDivisor,
				instanceData.m_pData,
				instanceData._numEntries);
			instancedLayout.GetElements().push_back(element);
			instanceDataSize += instanceData._Size * instanceData._numEntries;
		}


#ifdef VCE_DEBUG
		unsigned int iNumEntriesCheck = -1;
		unsigned int iNumElements = 0;
		for (BufferElement& instanceData : m_axInstanceData) {
			if (iNumEntriesCheck == -1)iNumEntriesCheck = instanceData._numEntries;
			else VCE_ASSERT(instanceData._numEntries == iNumEntriesCheck, "don't support varying number of entries")
				VCE_ASSERT(instanceData.m_uDivisor == 1, "need to implement support for different divisors");
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
			for (BufferElement& instanceData : m_axInstanceData) {
				int offset = elementOffsets[iCurrentElement];
				memcpy(pData + iDataOffset,
					(char*)instanceData.m_pData + offset,
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
		for (BufferElement& instanceData : m_axInstanceData) {
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