#pragma once

#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Window.h"
#include "VeryCoolEngine/Layers/Layer.h"
#include "VeryCoolEngine/Events/ApplicationEvent.h"
#include "VeryCoolEngine/Events/KeyEvent.h"
#include "VeryCoolEngine/Events/MouseEvent.h"

#include <imgui.h>

namespace VeryCoolEngine {

class VCE_API ImGuiLayer : public Layer{
public:
	ImGuiLayer();
	~ImGuiLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnImGuiRender() override;

	void Begin();
	void End();

private:
	float _time = 0;
};

}

