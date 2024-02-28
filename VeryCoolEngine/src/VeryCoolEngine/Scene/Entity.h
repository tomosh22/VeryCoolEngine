#pragma once
#include "Scene.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include <string>
#include "VeryCoolEngine/PlatformTypes.h"

namespace VeryCoolEngine {
	
	class Entity
	{
	public:
		Entity() = delete;
		Entity(Scene* pxScene, const std::string& strName);
		Entity(Scene* pxScene, GUID xGUID, GUID xParentGUID, const std::string& strName);

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
		TransformComponent& AddComponent(const std::string& strName) {
			return m_pxParentScene->m_xRegistry.emplace<TransformComponent>(m_xEntity, strName);
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
		
		const GUID GetGuid() const { return m_xGuid; }
		class Scene* m_pxParentScene;

		void Serialize(std::ofstream& xOut);

		GUID m_xParentEntityGUID;
	private:
		EntityID m_xEntity;
		
		GUID m_xGuid;
		
	};

}

