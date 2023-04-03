#pragma once

#include "Event.h"

namespace VeryCoolEngine {
	
	class VCE_API KeyEvent : public Event {
		friend class KeyPressedEvent;
		friend class KeyReleasedEvent;
	public:
		inline int GetKeyCode() const { return _keyCode; }
		int GetCategoryBitMask() const { return EventCategoryInput | EventCategoryKeyboard; }
	private:
		KeyEvent(int keyCode) : _keyCode(keyCode) {};
		int _keyCode;
	};



	class VCE_API KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(int keyCode, int repeatCount)
			: KeyEvent(keyCode), _repeatCount(repeatCount) {}
		inline int GetRepeatCount() const { return _repeatCount; }

		EventType GetType() const override { return EventType::KeyPressed;};
		
		std::string GetName() const override {
			std::string name = "KeyPressed " + std::to_string(_keyCode);
			return name;
		};
	private:
		int _repeatCount;
	};


	class VCE_API KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(int keyCode)
			: KeyEvent(keyCode) {}

		EventType GetType() const override { return EventType::KeyReleased; };
		std::string GetName() const override {
			std::string name = "KeyReleased " + std::to_string(_keyCode);
			return name;
		};
	};
}