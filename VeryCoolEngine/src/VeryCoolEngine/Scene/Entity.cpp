#include "vcepch.h"
#include "Entity.h"
#include "Scene.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ScriptComponent.h"

namespace VeryCoolEngine {

	Entity::Entity(Scene* pxScene) : m_pxParentScene(pxScene){
		m_xEntity = m_pxParentScene->m_xRegistry.create();
		AddComponent<TransformComponent>();
		pxScene->m_xEntityMap.insert({ m_xGuid.m_uGuid, *this });
	}

	Entity::Entity(Scene* pxScene, GUID xGuid) : m_pxParentScene(pxScene), m_xGuid(xGuid) {
		m_xEntity = m_pxParentScene->m_xRegistry.create();
		AddComponent<TransformComponent>();
		pxScene->m_xEntityMap.insert({ m_xGuid.m_uGuid, *this });
	}

	void Entity::Serialize(std::ofstream& xOut) {
		xOut << m_xGuid.m_uGuid << '\n';
		if (HasComponent<TransformComponent>())
			GetComponent<TransformComponent>().Serialize(xOut);
		if (HasComponent<ColliderComponent>())
			GetComponent<ColliderComponent>().Serialize(xOut);
		if (HasComponent<ModelComponent>())
			GetComponent<ModelComponent>().Serialize(xOut);
		if (HasComponent<ScriptComponent>())
			GetComponent<ScriptComponent>().Serialize(xOut);
		
	}
}