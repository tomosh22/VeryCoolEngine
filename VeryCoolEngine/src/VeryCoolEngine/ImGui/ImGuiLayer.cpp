#include "vcepch.h"
#include "ImGuiLayer.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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

		ImGui_ImplGlfw_InitForOpenGL(window,true);
		ImGui_ImplOpenGL3_Init();
	}

	void ImGuiLayer::Begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::OnImGuiRender() {
		Application* app = Application::GetInstance();
		//ImGui::ShowDemoWindow();
		std::string cameraText = std::string("Camera ") + (app->_mouseEnabled ? "enabled" : "disabled") + ". Q to toggle.";
		ImGui::Text(cameraText.c_str());

		const glm::ivec3& camPos = app->_Camera.GetPosition();
		std::string camPosText = std::to_string(camPos.x) + " " + std::to_string(camPos.y) + " " + std::to_string(camPos.z);
		ImGui::Text(camPosText.c_str());
		if (ImGui::TreeNode("Point Lights")) {
			int lightIndex = 1;
			for (Renderer::Light& light : app->_lights)
			{
				std::string label = "Light" + std::to_string(lightIndex++);
				ImGui::ColorEdit4(label.c_str(), &light.r);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Ambient Occlusion")) {
			Application* app = Application::GetInstance();
			ImGui::Checkbox("Enabled", &(app->_aoEnabled));
			ImGui::TreePop();
		}
		//ImGui::SliderFloat3("Pos", &app->_pMesh->transform._position[0],-10,10);
		//ImGui::SliderFloat3("Rotation", &app->_pMesh->transform._roll,0,360);

		

		
	}

	void ImGuiLayer::End() {
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
	}

	void ImGuiLayer::OnDetach() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}