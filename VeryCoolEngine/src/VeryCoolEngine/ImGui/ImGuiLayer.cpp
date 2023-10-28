#include "vcepch.h"
#include "ImGuiLayer.h"
#ifdef VCE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#ifdef VCE_OPENGL
#include <glad/glad.h>
#endif

#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif
#include "backends/imgui_impl_glfw.h"
#ifdef VCE_OPENGL
#include "backends/imgui_impl_opengl3.h"
#elif defined(VCE_VULKAN)
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "backends/imgui_impl_vulkan.h"
#endif

namespace VeryCoolEngine {

	ImGuiLayer::ImGuiLayer() : Layer("ImGui Layer") {

	}

	ImGuiLayer::~ImGuiLayer() {

	}

	void ImGuiLayer::OnAttach() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		GLFWwindow* window = (GLFWwindow*)(Application::GetInstance()->GetWindow().GetNativeWindow());
#ifdef VCE_OPENGL
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
#elif defined(VCE_VULKAN)
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = pxRenderer->m_instance;
		init_info.PhysicalDevice = pxRenderer->m_physicalDevice;
		init_info.Device = pxRenderer->m_device;
		init_info.QueueFamily = pxRenderer->FindQueueFamilies(VulkanRenderer::GetInstance()->m_physicalDevice).graphicsFamily;
		init_info.Queue = pxRenderer->m_graphicsQueue;
		init_info.DescriptorPool = pxRenderer->m_descriptorPool;
		init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
		ImGui_ImplVulkan_Init(&init_info, dynamic_cast<VulkanRenderPass*>(Application::GetInstance()->m_pxRenderPass)->m_xRenderPass);

		vk::CommandBuffer xCmd = pxRenderer->BeginSingleUseCmdBuffer();
		ImGui_ImplVulkan_CreateFontsTexture(xCmd);
		pxRenderer->EndSingleUseCmdBuffer(xCmd);

		pxRenderer->m_device.waitIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
#endif
	}

	void ImGuiLayer::Begin() {
#ifdef VCE_OPENGL
		ImGui_ImplOpenGL3_NewFrame();
#elif defined(VCE_VULKAN)
		ImGui_ImplVulkan_NewFrame();
#endif
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::OnImGuiRender() {
		Application* app = Application::GetInstance();
		std::string cameraText = std::string("Camera ") + (app->_mouseEnabled ? "enabled" : "disabled") + ". Q to toggle.";
		ImGui::Text(cameraText.c_str());

		const glm::ivec3& camPos = app->_Camera.GetPosition();
		std::string camPosText = "Camera Position: " + std::to_string(camPos.x) + " " + std::to_string(camPos.y) + " " + std::to_string(camPos.z);
		ImGui::Text(camPosText.c_str());

		const glm::vec3& camDir = app->_Camera.ViewDirection();
		std::string camDirText = "Camera View Direction: " + std::to_string(camDir.x) + " " + std::to_string(camDir.y) + " " + std::to_string(camDir.z);
		ImGui::Text(camDirText.c_str());

		if (ImGui::TreeNode("Point Lights")) {
			int lightIndex = 0;
			for (Renderer::Light& light : app->_lights)
			{
				std::string labelPos = "Light" + std::to_string(lightIndex) + " Position";
				ImGui::DragFloat3(labelPos.c_str(), &light.x);
				std::string labelCol = "Light" + std::to_string(lightIndex++) + " Colour";
				ImGui::ColorEdit4(labelCol.c_str(), &light.r);
			}
			ImGui::TreePop();
		}
		ImGui::Checkbox("Use Bumpmap", &app->_pRenderer->m_bUseBumpMaps);

		//do things

		ImGui::Render();

	}

	void ImGuiLayer::End() {
#ifdef VCE_OPENGL
		ImGuiIO& io = ImGui::GetIO();
		Application* app = Application::GetInstance();
		io.DisplaySize = ImVec2((float)app->GetWindow().GetWidth(), (float)app->GetWindow().GetHeight());

		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

#elif defined(VCE_VULKAN)
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
#endif

	}

	void ImGuiLayer::OnDetach() {
#ifdef VCE_OPENGL
		ImGui_ImplOpenGL3_Shutdown();
#elif defined(VCE_VULKAN)
		ImGui_ImplVulkan_Shutdown();
#endif
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}