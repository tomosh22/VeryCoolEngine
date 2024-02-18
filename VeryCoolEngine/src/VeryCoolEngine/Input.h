#pragma once

#include "VeryCoolEngine/core.h"
#include <set>

namespace VeryCoolEngine {
	class Input {
	public:
		static bool WasKeyPressed(int keyCode) { return s_xPressedKeys.find(keyCode) != s_xPressedKeys.end(); }
		static bool IsKeyDown(int keyCode) { return _sInstance->IsKeyDownImpl(keyCode); }
		static bool IsKeyReleased(int keyCode) { return _sInstance->IsKeyReleasedImpl(keyCode); }
		static bool IsMouseButtonPressed(int button) { return _sInstance->IsMouseButtonPressedImpl(button); }
		static std::pair<double, double> GetMousePos() { return _sInstance->GetMousePosImpl(); }

		static void ResetPressedKeys();

		static std::set<int> s_xPressedKeys;
	protected:
		virtual bool IsKeyDownImpl(int keyCode) = 0;
		virtual bool IsKeyReleasedImpl(int keyCode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<double, double> GetMousePosImpl() = 0;
	private:
		static Input* _sInstance;
	};
}