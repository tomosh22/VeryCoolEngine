#include "vcepch.h"
#include "VulkanMesh.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {

	static vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType t) {
		switch (t) {
		case ShaderDataType::Float: return vk::Format::eR32Sfloat;
		case ShaderDataType::Float2: return vk::Format::eR32G32Sfloat;
		case ShaderDataType::Float3: return vk::Format::eR32G32B32Sfloat;
		case ShaderDataType::Float4: return vk::Format::eR32G32B32A32Sfloat;
		case ShaderDataType::Int: return vk::Format::eR32Sint;
		case ShaderDataType::Int2: return vk::Format::eR32G32Sint;
		case ShaderDataType::Int3: return vk::Format::eR32G32B32Sint;
		case ShaderDataType::Int4: return vk::Format::eR32G32B32A32Sint;
		case ShaderDataType::UInt: return vk::Format::eR32Uint;
		case ShaderDataType::UInt2: return vk::Format::eR32G32Uint;
		case ShaderDataType::UInt3: return vk::Format::eR32G32B32Uint;
		case ShaderDataType::UInt4: return vk::Format::eR32G32B32A32Uint;
		}
	}

    void VulkanMesh::PlatformInit()
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

		layout.CalculateOffsetsAndStrides();


		uint32_t uBindPoint = 0;
		uint32_t uTotalSize = 0;
		for (BufferElement& element : layout.GetElements()) {
			

			vk::VertexInputAttributeDescription xAttrDesc = vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(uBindPoint)
				.setOffset(element._Offset)
				.setFormat(ShaderDataTypeToVulkanFormat(element._Type));
			m_axAttrDescs.push_back(xAttrDesc);
			uBindPoint++;
			uTotalSize += element._Offset;
		}

		vk::VertexInputBindingDescription xBindDesc = vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(float) * numFloats)
			.setInputRate(vk::VertexInputRate::eVertex);
		m_axBindDescs.push_back(xBindDesc);

		m_pxVertexBuffer = new VulkanVertexBuffer(verts, numVerts * numFloats * sizeof(float));

		m_pxIndexBuffer = new VulkanIndexBuffer(indices, sizeof(unsigned int) * numIndices);
    }

    void VulkanMesh::SetVertexArray(VertexArray* vertexArray)
    {
    }

    VertexBuffer* VulkanMesh::CreateInstancedVertexBuffer()
    {
        return nullptr;
    }

}