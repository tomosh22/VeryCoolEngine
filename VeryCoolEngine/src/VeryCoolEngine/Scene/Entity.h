#pragma once
#include "Scene.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ModelComponent.h"

#include <string>

namespace VeryCoolEngine {
	class Material;

	class Entity
	{
	public:
		Entity();

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			VCE_ASSERT(!HasComponent<T>(), "Already has this component");
			return m_pxParentScene->m_xRegistry.emplace<T>(m_xEntity, std::forward<Args>(args)..., GetComponent<TransformComponent>());
		}

		template<>
		TransformComponent& AddComponent() {
			return m_pxParentScene->m_xRegistry.emplace<TransformComponent>(m_xEntity);
		}

		template<typename T>
		bool HasComponent() {
			return m_pxParentScene->m_xRegistry.all_of<T>(m_xEntity);
		}

		template<typename T>
		T& GetComponent() {
			VCE_ASSERT(HasComponent<T>(), "Doesn't have this component");
			return m_pxParentScene->m_xRegistry.get<T>(m_xEntity);
		}

		template<typename T>
		void RemoveComponent() {
			VCE_ASSERT(HasComponent<T>(), "Doesn't have this component");
			m_pxParentScene->m_xRegistry.remove<T>(m_xEntity);
		}

		

	private:
		entt::entity m_xEntity;
		Scene* m_pxParentScene;
	};

}

