#pragma once
#include "VeryCoolEngine/Renderer/Camera.h"
#include "single_include/entt/entt.hpp"

namespace VeryCoolEngine {
	class ColliderComponent;
	using EntityRegistry = entt::registry;
	using EntityID = entt::entity;
	class Scene
	{
	public:
		virtual void Reset() = 0;

		template<typename T>
		T& GetComponentFromEntity(EntityID xID) {
			VCE_ASSERT(EntityHasComponent<T>(xID), "Doesn't have this component");
			return m_xRegistry.get<T>(xID);
		}

		template<typename T>
		bool EntityHasComponent(EntityID xID) const {
			return m_xRegistry.all_of<T>(xID);
		}

		template<typename T>
		std::vector<T*> GetAllOfComponentType() {
			std::vector<T*> xRet;
			auto view = m_xRegistry.view<T>();
			for (auto [xEntity, xComponent] : view.each())
				xRet.push_back(&xComponent);
			return xRet;
		}

		
		std::vector<ColliderComponent*> GetAllColliderComponents();

		Camera m_xEditorCamera;
		Camera m_xGameCamera;
	private:
		friend class Entity;
		EntityRegistry m_xRegistry;
	};

}

