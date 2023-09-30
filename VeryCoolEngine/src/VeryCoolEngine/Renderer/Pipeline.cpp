#include "vcepch.h"
#include "Pipeline.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace VeryCoolEngine {
	

	Pipeline* Pipeline::Create(Shader* pxShader, BufferLayout xLayout, MeshTopolgy xTopology, const std::vector<ManagedUniformBuffer*>& apxUBOs, RenderPass* xRenderPass)
	{
#ifdef VCE_VULKAN
		return new VulkanPipeline(pxShader, xLayout, xTopology, apxUBOs, xRenderPass);
#endif
	}

}