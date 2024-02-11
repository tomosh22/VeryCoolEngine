#pragma once
#include "Scene.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include <string>

namespace VeryCoolEngine {

	class Entity
	{
	public:
		Entity() = delete;
		Entity(class Scene* pxScene);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			VCE_ASSERT(!HasComponent<T>(), "Already has this component");
			return m_pxParentScene->m_xRegistry.emplace<T>(m_xEntity, std::forward<Args>(args)..., GetComponent<TransformComponent>(), this);
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args) {
			if (HasComponent<T>())
				RemoveComponent<T>();
			return m_pxParentScene->m_xRegistry.emplace<T>(m_xEntity, std::forward<Args>(args)..., GetComponent<TransformComponent>(), this);
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

		EntityID GetEntityID() { return m_xEntity; }
		

	private:
		EntityID m_xEntity;
		Scene* m_pxParentScene;
	};

}

