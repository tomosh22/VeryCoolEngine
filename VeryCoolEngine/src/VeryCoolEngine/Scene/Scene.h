#pragma once
#include "VeryCoolEngine/Renderer/Camera.h"
#include "single_include/entt/entt.hpp"

namespace VeryCoolEngine {
	using EntityRegistry = entt::registry;
	class Scene
	{
	public:
		void Reset();

		template<typename T>
		std::vector<T*> GetAllOfComponentType() {
			std::vector<T*> xRet;
			auto view = m_xRegistry.view<T>();
			for (auto [xEntity, xComponent] : view.each())
				xRet.push_back(&xComponent);
			return xRet;
		}

		Camera m_xEditorCamera;
		Camera m_xGameCamera;
	private:
		friend class Entity;
		EntityRegistry m_xRegistry;
	};

}

