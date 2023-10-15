#include "vcepch.h"
#include "Pipeline.h"

namespace VeryCoolEngine {
	

	

	Pipeline* Pipeline::Create(Shader* pxShader, BufferLayout* xLayout, MeshTopolgy xTopology, std::vector<ManagedUniformBuffer**> apxUBOs, std::vector<Texture2D**> apxTextures, RenderPass** xRenderPass)
	{
#ifdef VCE_VULKAN
		//return new VulkanPipeline(pxShader, xLayout, xTopology, apxUBOs, apxTextures, xRenderPass);
#endif
		return nullptr;
	}

}