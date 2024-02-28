#include "vcepch.h"
#include "Entity.h"
#include "Scene.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ScriptComponent.h"

namespace VeryCoolEngine {

	Entity::Entity(Scene* pxScene, const std::string& strName) : m_pxParentScene(pxScene){
		m_xEntity = m_pxParentScene->m_xRegistry.create();
		AddComponent<TransformComponent>(strName);
		pxScene->m_xEntityMap.insert({ m_xGuid.m_uGuid, *this });
	}

	Entity::Entity(Scene* pxScene, GUID xGUID, GUID xParentGUID, const std::string& strName) : m_pxParentScene(pxScene), m_xGuid(xGUID), m_xParentEntityGUID(xParentGUID) {
		m_xEntity = m_pxParentScene->m_xRegistry.create();
		AddComponent<TransformComponent>(strName);
		pxScene->m_xEntityMap.insert({ m_xGuid.m_uGuid, *this });
	}

	void Entity::Serialize(std::ofstream& xOut) {
		xOut << m_xGuid.m_uGuid << '\n';
		xOut << GetComponent<TransformComponent>().m_strName << '\n';
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