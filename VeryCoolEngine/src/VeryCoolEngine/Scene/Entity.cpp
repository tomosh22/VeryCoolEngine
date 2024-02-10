#include "vcepch.h"
#include "Entity.h"
#include "Scene.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ModelComponent.h"

namespace VeryCoolEngine {

	Entity::Entity(){
		m_pxParentScene = Application::GetInstance()->m_pxCurrentScene;
		m_xEntity = m_pxParentScene->m_xRegistry.create();
		AddComponent<TransformComponent>();
		//AddComponent<ModelComponent>();
	}
}