#pragma once

#include "Event.h"

namespace VeryCoolEngine {
	
	class VCE_API WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height) : _width(width), _height(height) {};

		inline float GetWidth() const { return _width; }
		inline float GetHeight() const { return _height; }

		EventType GetType() const override { return EventType::WindowResize; };
		std::string GetName() const override { return "WindowResize"; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	private:
		unsigned int _width, _height;
	};



	class VCE_API WindowCloseEvent : public Event {
	public:
		WindowCloseEvent(unsigned int width, unsigned int height) {};

		EventType GetType() const override { return EventType::WindowClose; };
		std::string GetName() const override { return "WindowClose"; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};


	class VCE_API AppTickEvent : public Event {
	public:
		AppTickEvent() {};

		EventType GetType() const override { return EventType::AppTick;};
		std::string GetName() const override { return "AppTick"; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};



	class VCE_API AppUpdateEvent : public Event {
	public:
		AppUpdateEvent() {};

		EventType GetType() const override { return EventType::AppUpdate; };
		std::string GetName() const override { return "AppUpdate"; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};

	class VCE_API AppRenderEvent : public Event {
	public:
		AppRenderEvent() {};

		EventType GetType() const override { return EventType::AppRender; };
		std::string GetName() const override { return "AppRender"; };

		int GetCategoryBitMask() const { return EventCategoryApplication; }
	};
}