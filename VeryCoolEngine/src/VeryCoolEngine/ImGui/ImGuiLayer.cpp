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

#include "reactphysics3d/reactphysics3d.h"

namespace VeryCoolEngine {


	ImGuiLayer::ImGuiLayer() : Layer("ImGui Layer") {

	}

	ImGuiLayer::~ImGuiLayer() {

	}

	void ImGuiLayer::OnAttach() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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
		init_info.DescriptorPool = pxRenderer->m_xImguiDescriptorPool;
		init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
		ImGui_ImplVulkan_Init(&init_info, dynamic_cast<VulkanRenderPass*>(Application::GetInstance()->m_pxImguiRenderPass)->m_xRenderPass);

		vk::CommandBuffer xCmd = pxRenderer->BeginSingleUseCmdBuffer();
		ImGui_ImplVulkan_CreateFontsTexture(xCmd);
		pxRenderer->EndSingleUseCmdBuffer(xCmd);

		pxRenderer->m_device.waitIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
#endif

		Application::GetInstance()->m_bImGuiInitialised = true;
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

#if 0
		ImGui::Begin("ImGui");


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
		ImGui::ColorEdit3("Override Normal", &app->_pRenderer->m_xOverrideNormal[0]);
		ImGui::Checkbox("Use Bumpmap", &app->_pRenderer->m_bUseBumpMaps);
		ImGui::Checkbox("Use Phong Tesselation", &app->_pRenderer->m_bUsePhongTess);
		ImGui::SliderFloat("Phong Tesselation Factor", &app->_pRenderer->m_fPhongTessFactor, -2, 20);
		ImGui::SliderInt("Tesselation Level", (int*)&app->_pRenderer->m_uTessLevel, 1, 64);

		ImGui::Image(VulkanRenderer::GetInstance()->m_axFramebufferTexDescSet[VulkanRenderer::GetInstance()->m_currentFrame], ImVec2(100, 100));

		ImGui::End();
#else
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		static bool bOpen = true;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		
		ImGui::Begin("DockSpace Demo", &bOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::Begin("Image");
		std::vector<vk::DescriptorSet>& xSets = VulkanRenderer::GetInstance()->m_axFramebufferTexDescSet;
		vk::DescriptorSet xSet = xSets[VulkanRenderer::GetInstance()->m_currentFrame];
		ImGui::Image(xSet, ImVec2(VCE_GAME_WIDTH, VCE_GAME_HEIGHT));
		ImGui::End();

		ImGui::Begin("ImGui");

		float fFrameRate = 1. / app->m_fDeltaTime;
		ImGui::Text(std::to_string(fFrameRate).c_str());

		std::string cameraText = std::string("Camera ") + (app->_mouseEnabled ? "enabled" : "disabled") + ". Q to toggle.";
		ImGui::Text(cameraText.c_str());

		const glm::ivec3& camPos = app->m_pxCurrentScene->m_xEditorCamera.GetPosition();
		std::string camPosText = "Camera Position: " + std::to_string(camPos.x) + " " + std::to_string(camPos.y) + " " + std::to_string(camPos.z);
		ImGui::Text(camPosText.c_str());

		const glm::vec3& camDir = app->m_pxCurrentScene->m_xEditorCamera.ViewDirection();
		std::string camDirText = "Camera View Direction: " + std::to_string(camDir.x) + " " + std::to_string(camDir.y) + " " + std::to_string(camDir.z);
		ImGui::Text(camDirText.c_str());

		if (ImGui::TreeNode("Point Lights")) {
			int lightIndex = 0;
			for (RendererAPI::Light& light : app->_lights)
			{
				std::string labelPos = "Light" + std::to_string(lightIndex) + " Position";
				ImGui::DragFloat3(labelPos.c_str(), &light.x);
				std::string labelCol = "Light" + std::to_string(lightIndex++) + " Colour";
				ImGui::ColorEdit4(labelCol.c_str(), &light.r);
			}
			ImGui::TreePop();
		}

		//#TO_TODO: reimplment this
#if 0
		if (ImGui::TreeNode("Models")) {
			int meshIndex = 0;
			if(app->m_pxSelectedModel != nullptr)
			{
				std::string labelPos = "Model" + std::to_string(meshIndex) + " Position";
				ImGui::DragFloat3(labelPos.c_str(), (float*)&app->m_pxSelectedModel->m_pxTransform->getPosition().x);
				std::string labelRot = "Model" + std::to_string(meshIndex) + " Rotation";
				ImGui::DragFloat3(labelRot.c_str(), (float*)&app->m_pxSelectedModel->m_pxTransform->getOrientation().x);//TODO: is this right?

				//TODO
				//std::string labelScale = "Mesh" + std::to_string(meshIndex++) + " Scale";
				//ImGui::DragFloat3(labelScale.c_str(), &mesh->m_xTransform._scale.x);
			}
			ImGui::TreePop();
		}
#endif


		ImGui::ColorEdit3("Override Normal", &app->_pRenderer->m_xOverrideNormal[0]);
		ImGui::Checkbox("Use Bumpmap", &app->_pRenderer->m_bUseBumpMaps);
		ImGui::Checkbox("Use Phong Tesselation", &app->_pRenderer->m_bUsePhongTess);
		ImGui::SliderFloat("Phong Tesselation Factor", &app->_pRenderer->m_fPhongTessFactor, -2, 20);
		ImGui::SliderInt("Tesselation Level", (int*)&app->_pRenderer->m_uTessLevel, 1, 64);
		ImGui::Checkbox("Animate", &app->_pRenderer->bAnimate);
		ImGui::SliderFloat("Animation Alpha", &app->_pRenderer->fAnimAlpha, 0,1);

		const char* aszItems[] = { "Editor", "Playing" };
		ImGui::Combo("combo", &app->m_eCurrentState, aszItems, IM_ARRAYSIZE(aszItems));

		if (ImGui::Button("Serialize Scene")) {
			app->m_pxCurrentScene->Serialize("TestScene.vcescene");
		}
		if (ImGui::Button("Reset Scene")) {
			app->m_bWantToResetScene = true;
		}

		ImGui::Text(std::to_string(app->_pRenderer->m_uNumDrawCalls).c_str());

		ImGui::End();

		/*if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
				ImGui::MenuItem("Padding", NULL, &opt_padding);
				ImGui::Separator();

				if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
				if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
				if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
				if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
				if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
				ImGui::Separator();

				if (ImGui::MenuItem("Close", NULL, false, &bOpen != NULL))
					bOpen = false;
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();

			
		}*/

		ImGui::End();
#endif

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
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
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