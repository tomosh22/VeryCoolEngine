#include "vcepch.h"
#include "VulkanMesh.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {

	vk::Format VulkanMesh::ShaderDataTypeToVulkanFormat(ShaderDataType t) {
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
		

		uint32_t uBindPoint = 0;
		uint32_t uTotalSize = 0;
		for (BufferElement& element : m_xBufferLayout->GetElements()) {
			

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
			.setStride(m_xBufferLayout->_Stride)
			.setInputRate(vk::VertexInputRate::eVertex);
		m_axBindDescs.push_back(xBindDesc);

		m_pxVertexBuffer = new VulkanVertexBuffer(verts, numVerts * m_xBufferLayout->_Stride);

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