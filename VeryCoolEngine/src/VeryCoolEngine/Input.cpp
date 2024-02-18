#include "vcepch.h"
#include "Input.h"

namespace VeryCoolEngine {
	 std::set<int> Input::s_xPressedKeys;

	 void Input::ResetPressedKeys() {
		 s_xPressedKeys.clear();
	 }
}