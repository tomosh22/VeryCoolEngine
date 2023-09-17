#include "vcepch.h"
#include "RenderContext.h"

#if 0
namespace VeryCoolEngine {
	RenderContext* RenderContext::Create() {
#ifdef VCE_OPENGL
		VCE_ASSERT(false, "shouldn't be creating a context for opengl")
#elif defined VCE_VULKAN
		return new VulkanContext();
#endif
	}

}
#endif