#include "vcepch.h"
#include "Scene.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
namespace VeryCoolEngine {
	void Scene::Reset() {
		
	}

	//#TO_TODO: don't like this
	std::vector<ColliderComponent*> Scene::GetAllColliderComponents() {
		std::vector<ColliderComponent*> xRet;
		for (ColliderComponent* pxCol : GetAllOfComponentType<ColliderComponent>())
			xRet.push_back(pxCol);
		return xRet;
	}
}