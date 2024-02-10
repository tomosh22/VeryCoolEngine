#include "vcepch.h"
#include "Entity.h"
#include "Scene.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ModelComponent.h"

namespace VeryCoolEngine {

	Entity::Entity(Scene* pxScene) : m_pxParentScene(pxScene){
		m_xEntity = m_pxParentScene->m_xRegistry.create();
		AddComponent<TransformComponent>();
		//AddComponent<ModelComponent>();
	}
}