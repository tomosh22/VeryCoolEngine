#pragma once
#include "vcepch.h"
#include "VeryCoolEngine/core.h"
#include "VeryCoolEngine/Events/Event.h"

namespace VeryCoolEngine {

	struct WindowProperties {
		std::string _title;
		unsigned int _width, _height;

		WindowProperties
		(const std::string& title = "Very Cool Engine",
			unsigned int width = 1280, unsigned int height = 720)
			: _title(title), _width(width), _height(height) {}
	};

	class VCE_API Window {
	public:
		using EventCallBackFunction = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual inline void SetEventCallback(const EventCallBackFunction& c) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool GetVSyncEnabled() const = 0;

		static Window* Create(const WindowProperties& p = WindowProperties());
	};
}

