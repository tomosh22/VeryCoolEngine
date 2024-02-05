#include "vcepch.h"
#include "Entity.h"
#include "Scene.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {

	Entity::Entity() {
		m_pxParentScene = Application::GetInstance()->m_pxCurrentScene;
		m_xEntity = m_pxParentScene->m_xRegistry.create();
	}

	template<typename T, typename... Args>
	T& Entity::AddComponent(Args&&... args) {
		VCE_ASSERT(HasComponent<T>(), "Already has this component");
		m_pxParentScene->m_xRegistry.emplace<T>(m_xEntity, std::forward<Args>(args));
	}

	template<typename T>
	bool Entity::HasComponent() {
		return m_pxParentScene->m_xRegistry.all_of<T>(m_xEntity);
	}

	template<typename T>
	T& Entity::GetComponent() {
		VCE_ASSERT(HasComponent<T>(), "Doesn't have this component");
		return m_pxParentScene->m_xRegistry.get<T>(m_xEntity);
	}

	template<typename T>
	void Entity::RemoveComponent() {
		VCE_ASSERT(HasComponent<T>(), "Doesn't have this component");
		m_pxParentScene->m_xRegistry.remove<T>(m_xEntity);
	}
}