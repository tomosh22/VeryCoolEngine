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
		for (BufferElement& element : m_xBufferLayout->GetElements()) {
			

			vk::VertexInputAttributeDescription xAttrDesc = vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(uBindPoint)
				.setOffset(element._Offset)
				.setFormat(ShaderDataTypeToVulkanFormat(element._Type));
			m_axAttrDescs.push_back(xAttrDesc);
			uBindPoint++;
		}

		vk::VertexInputBindingDescription xBindDesc = vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(m_xBufferLayout->_Stride)
			.setInputRate(vk::VertexInputRate::eVertex);
		m_axBindDescs.push_back(xBindDesc);

		m_pxVertexBuffer = new VulkanVertexBuffer(verts, numVerts * m_xBufferLayout->_Stride);

		m_pxIndexBuffer = new VulkanIndexBuffer(indices, sizeof(unsigned int) * numIndices);

		if (_instanceData.size() > 0) {
			m_pxInstanceBuffer = dynamic_cast<VulkanVertexBuffer*>(CreateInstancedVertexBuffer());
			BufferLayout& xInstanceLayout = const_cast<BufferLayout&>(m_pxInstanceBuffer->GetLayout());
			for (BufferElement& element : xInstanceLayout.GetElements()) {


				vk::VertexInputAttributeDescription xInstanceAttrDesc = vk::VertexInputAttributeDescription()
					.setBinding(1)
					.setLocation(uBindPoint)
					.setOffset(element._Offset)
					.setFormat(ShaderDataTypeToVulkanFormat(element._Type));
				m_axAttrDescs.push_back(xInstanceAttrDesc);
				uBindPoint++;
			}

			vk::VertexInputBindingDescription xInstanceBindDesc = vk::VertexInputBindingDescription()
				.setBinding(1)
				.setStride(xInstanceLayout._Stride)
				.setInputRate(vk::VertexInputRate::eInstance);
			m_axBindDescs.push_back(xInstanceBindDesc);
		}

		m_xVertexInputState = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(m_axBindDescs.size())
			.setPVertexBindingDescriptions(m_axBindDescs.data())
			.setVertexAttributeDescriptionCount(m_axAttrDescs.size())
			.setPVertexAttributeDescriptions(m_axAttrDescs.data());

    }


	void VulkanMesh::BindToCmdBuffer(vk::CommandBuffer& xCmdBuffer) const {
		vk::DeviceSize offsets[] = { 0 };
		xCmdBuffer.bindVertexBuffers(0, 1, &m_pxVertexBuffer->m_pxVertexBuffer->m_xBuffer, offsets);

		if (m_pxIndexBuffer != nullptr) {
			xCmdBuffer.bindIndexBuffer(m_pxIndexBuffer->m_pxIndexBuffer->m_xBuffer, 0, vk::IndexType::eUint32);
		}

		if (m_pxInstanceBuffer != nullptr) {
			xCmdBuffer.bindVertexBuffers(1, 1, &m_pxInstanceBuffer->m_pxVertexBuffer->m_xBuffer, offsets);
		}
	}

    void VulkanMesh::SetVertexArray(VertexArray* vertexArray)
    {
    }

    VertexBuffer* VulkanMesh::CreateInstancedVertexBuffer()
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