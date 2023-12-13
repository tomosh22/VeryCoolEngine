#include "vcepch.h"
#include "VulkanMesh.h"
#include "VeryCoolEngine/Application.h"

#include "VulkanDescriptorSetLayoutBuilder.h"

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
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();

		if(m_pxTexture != nullptr)m_pxTexture->PlatformInit();
		if(m_pxBumpMap != nullptr)m_pxBumpMap->PlatformInit();
		if(m_pxRoughnessTex != nullptr)m_pxRoughnessTex->PlatformInit();
		if(m_pxMetallicTex != nullptr)m_pxMetallicTex->PlatformInit();
		if(m_pxHeightmapTex != nullptr)m_pxHeightmapTex->PlatformInit();

		uint32_t uBindPoint = 0;
		for (BufferElement& element : m_pxBufferLayout->GetElements()) {
			

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
			.setStride(m_pxBufferLayout->_Stride)
			.setInputRate(vk::VertexInputRate::eVertex);
		m_axBindDescs.push_back(xBindDesc);

		m_pxVertexBuffer = new VulkanVertexBuffer(m_pVerts, m_uNumVerts * m_pxBufferLayout->_Stride);

		m_pxIndexBuffer = new VulkanIndexBuffer(m_puIndices, sizeof(unsigned int) * m_uNumIndices);

		if (m_axInstanceData.size() > 0) {
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

		if (m_xTexDescSpec.m_aeSamplerStages.size()) {
			m_xTexDescSetLayout = VulkanDescriptorSetLayoutBuilder::FromSpecification(m_xTexDescSpec);
			m_xTexDescSet = pxRenderer->CreateDescriptorSet(m_xTexDescSetLayout, pxRenderer->GetDescriptorPool());

			if(m_pxTexture != nullptr) 
				pxRenderer->UpdateImageDescriptor(m_xTexDescSet, 0, 0, dynamic_cast<VulkanTexture2D*>(m_pxTexture)->m_xImageView, dynamic_cast<VulkanTexture2D*>(m_pxTexture)->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);
			if (m_pxBumpMap != nullptr)
				pxRenderer->UpdateImageDescriptor(m_xTexDescSet, 1, 0, dynamic_cast<VulkanTexture2D*>(m_pxBumpMap)->m_xImageView, dynamic_cast<VulkanTexture2D*>(m_pxBumpMap)->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);
			if (m_pxRoughnessTex != nullptr)
				pxRenderer->UpdateImageDescriptor(m_xTexDescSet, 2, 0, dynamic_cast<VulkanTexture2D*>(m_pxRoughnessTex)->m_xImageView, dynamic_cast<VulkanTexture2D*>(m_pxRoughnessTex)->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);
			if (m_pxMetallicTex != nullptr)
				pxRenderer->UpdateImageDescriptor(m_xTexDescSet, 3, 0, dynamic_cast<VulkanTexture2D*>(m_pxMetallicTex)->m_xImageView, dynamic_cast<VulkanTexture2D*>(m_pxMetallicTex)->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);
			if (m_pxHeightmapTex != nullptr)
				pxRenderer->UpdateImageDescriptor(m_xTexDescSet, 4, 0, dynamic_cast<VulkanTexture2D*>(m_pxHeightmapTex)->m_xImageView, dynamic_cast<VulkanTexture2D*>(m_pxHeightmapTex)->m_xSampler, vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		if (m_uNumBones) {
			uint64_t uSize = m_xBoneMats.size() * sizeof(glm::mat4);
			//TODO: does this want to be device local?
#if 0
			VulkanBuffer pxStagingBuffer = VulkanBuffer(uSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
			void* pData = xDevice.mapMemory(pxStagingBuffer.m_xDeviceMem, 0, uSize);
			memcpy(pData, m_xBoneMats.data(), uSize);
			xDevice.unmapMemory(pxStagingBuffer.m_xDeviceMem);

			m_pxBoneBuffer = new VulkanBuffer(uSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

			VulkanBuffer::CopyBufferToBuffer(&pxStagingBuffer, m_pxBoneBuffer, uSize);
#endif
			m_pxBoneBuffer = new VulkanBuffer(uSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			vk::Device xDevice = VulkanRenderer::GetInstance()->GetDevice();
			void* pData = xDevice.mapMemory(m_pxBoneBuffer->m_xDeviceMem, 0, uSize);
			memcpy(pData, m_xBoneMats.data(), uSize);
			xDevice.unmapMemory(m_pxBoneBuffer->m_xDeviceMem);

		}

		m_bInitialised = true;
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
		unsigned int iNumElements = m_axInstanceData.size();
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