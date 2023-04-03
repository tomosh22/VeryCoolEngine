#pragma once

#include "Event.h"

namespace VeryCoolEngine {
	
	class VCE_API MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float x, float y) : _mouseX(x), _mouseY(y) {};

		inline float GetX() const { return _mouseX; }
		inline float GetY() const { return _mouseY; }

		EventType GetType() const override { return EventType::MouseMoved; };
		std::string GetName() const override { return "MouseMoved"; };

		int GetCategoryBitMask() const { return EventCategoryInput | EventCategoryMouse; }

		
	private:
		float _mouseX, _mouseY;
	};



	class VCE_API MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float scrollX, float scrollY)
			: _scrollX(scrollX), _scrollY(scrollY) {}

		inline float GetX() const { return _scrollX; }
		inline float GetY() const { return _scrollY; }


		EventType GetType() const override { return EventType::MouseScrolled;};
		std::string GetName() const override { return "MouseScrolled"; };

		int GetCategoryBitMask() const { return EventCategoryInput | EventCategoryMouse; }
	private:
		float _scrollX, _scrollY;
	};


	class VCE_API MouseButtonEvent : public Event {
		friend class MouseButtonPressedEvent;
		friend class MouseButtonReleasedEvent;
	public:
		inline int GetMouseButtonCode() const { return _mouseButtonCode; }
		int GetCategoryBitMask() const { return EventCategoryInput | EventCategoryMouse; }
	private:
		MouseButtonEvent(int mouseButtonCode) : _mouseButtonCode(mouseButtonCode) {};
		int _mouseButtonCode;
	};



	class VCE_API MouseButtonPressedEvent : public MouseButtonEvent {
	public:
		MouseButtonPressedEvent(int MouseButtonCode)
			: MouseButtonEvent(MouseButtonCode){}

		EventType GetType() const override { return EventType::MouseButtonPressed;};
		std::string GetName() const override { return "MouseButtonPressed"; };
	};


	class VCE_API MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
		MouseButtonReleasedEvent(int MouseButtonCode)
			: MouseButtonEvent(MouseButtonCode) {}

		EventType GetType() const override { return EventType::MouseButtonReleased; };
		std::string GetName() const override { return "MouseButtonReleased"; };
	};
}