#pragma once
#include "VeryCoolEngine/Input.h"


namespace VeryCoolEngine {
	class WindowsInput : public Input{
	protected:
		bool IsKeyDownImpl(int keyCode) override;
		bool IsKeyReleasedImpl(int keyCode) override;
		bool IsMouseButtonPressedImpl(int button) override;
		std::pair<double, double> GetMousePosImpl() override;
	};
}



