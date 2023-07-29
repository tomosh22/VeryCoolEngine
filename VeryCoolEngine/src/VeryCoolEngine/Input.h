#pragma once

#include "VeryCoolEngine/core.h"

namespace VeryCoolEngine {
	class VCE_API Input {
	public:
		static bool IsKeyPressed(int keyCode) { return _sInstance->IsKeyPressedImpl(keyCode); }
		static bool IsMouseButtonPressed(int button) { return _sInstance->IsMouseButtonPressedImpl(button); }
		static std::pair<double, double> GetMousePos() { return _sInstance->GetMousePosImpl(); }
	protected:
		virtual bool IsKeyPressedImpl(int keyCode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<double, double> GetMousePosImpl() = 0;
	private:
		static Input* _sInstance;
	};
}