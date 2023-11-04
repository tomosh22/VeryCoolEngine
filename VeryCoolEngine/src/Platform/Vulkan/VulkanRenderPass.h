#pragma once
#include "VeryCoolEngine/Renderer/RenderPass.h"
#include "VulkanRenderer.h"

namespace VeryCoolEngine {
	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass();
		~VulkanRenderPass() override {
			VulkanRenderer::GetInstance()->GetDevice().destroyRenderPass(m_xRenderPass, nullptr);
		};
		vk::RenderPass m_xRenderPass;

		static vk::RenderPass GBufferRenderPass();
		static vk::RenderPass ImguiRenderPass();//imgui doesn't use depth buffer
	};

}