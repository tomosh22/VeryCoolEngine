#pragma once

#include "VeryCoolEngine/Core.h"
#include "VeryCoolEngine/Events/Event.h"

namespace VeryCoolEngine {

	class VCE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return _debugName; }
	protected:
		std::string _debugName;
	};

}