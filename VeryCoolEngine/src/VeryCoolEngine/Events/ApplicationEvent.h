#pragma once

#include "Event.h"

namespace VeryCoolEngine {
	
	class VCE_API WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height) : _width(width), _height(height) {};

		inline float GetWidth() const { return (float)_width; }
		inline float GetHeight() const { return (float)_height; }

		EventType GetType() const override { return EventType::WindowResize; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	private:
		unsigned int _width, _height;
	};



	class VCE_API WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() {};

		EventType GetType() const override { return EventType::WindowClose; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};


	class VCE_API AppTickEvent : public Event {
	public:
		AppTickEvent() {};

		EventType GetType() const override { return EventType::AppTick;};

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};



	class VCE_API AppUpdateEvent : public Event {
	public:
		AppUpdateEvent() {};

		EventType GetType() const override { return EventType::AppUpdate; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};

	class VCE_API AppRenderEvent : public Event {
	public:
		AppRenderEvent() {};

		EventType GetType() const override { return EventType::AppRender; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};
}