#pragma once
#include "VeryCoolEngine/Renderer/RenderPass.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace VeryCoolEngine {
	class VulkanRenderPass : public RenderPass
	{
	public:
		~VulkanRenderPass() override = default;
		vk::RenderPass m_xRenderPass;
	};

}