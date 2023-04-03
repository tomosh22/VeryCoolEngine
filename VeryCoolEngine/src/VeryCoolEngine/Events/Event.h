#pragma once
#include "vcepch.h"
#include "VeryCoolEngine/core.h"



namespace VeryCoolEngine {
	
	enum class EventType {
		None,
		WindowClose,WindowResize, WindowFocus, WindowLostFocus,WindowMoved,
		AppTick,AppUpdate,AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory {
		None = 0,
		EventCategoryApplication = 1 << 0,
		EventCategoryInput = 1 << 1,
		EventCategoryKeyboard = 1 << 2,
		EventCategoryMouse = 1 << 3,
		EventCategoryButton = 1 << 4
	};

	class VCE_API Event {
		friend class EventDispatcher;
	public:
		virtual EventType GetType() const = 0;
		virtual std::string GetName() const = 0;
		virtual int GetCategoryBitMask() const = 0;
		inline bool IsInCategory(EventCategory c) const {
			return GetCategoryBitMask() & c;
		}
	private:
		bool _handled = false;
	};

	class EventDispatcher {
		template<typename T>
		using EventFunction = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) : _event(event) {}

		template<typename T>
		bool Dispatch(EventFunction<T> func) {
			T* castEventPtr = (T*)&_event;
			_event._handled = func(*castEventPtr);
			return _event._handled;
		}

	private:
		Event& _event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.GetName();
	}
}