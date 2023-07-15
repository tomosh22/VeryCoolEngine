#pragma once

#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Window.h"
#include "VeryCoolEngine/Layers/Layer.h"
#include "VeryCoolEngine/Events/ApplicationEvent.h"
#include "VeryCoolEngine/Events/KeyEvent.h"
#include "VeryCoolEngine/Events/MouseEvent.h"

#include "../../imgui/imgui.h"
#include "../src/Platform/OpenGL/imgui_impl_opengl3.h"

namespace VeryCoolEngine {

class VCE_API ImGuiLayer : public Layer{
public:
	ImGuiLayer();
	~ImGuiLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate() override;
	void OnEvent(Event& event) override;

private:
	bool OnMouseMovedEvent(MouseMovedEvent& event);
	bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
	bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event);
	bool OnMouseScrolledEvent(MouseScrolledEvent& event);
	bool OnKeyPressedEvent(KeyPressedEvent& event);
	bool OnKeyReleasedEvent(KeyReleasedEvent& event);
	//bool OnKeyTypedEvent(KeyTypedEvent& event);
	bool OnWindowResizeEvent(WindowResizeEvent& event);

	float _time = 0;
};

}

