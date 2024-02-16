#include "vcepch.h"
#include "Entity.h"
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

	void Scene::Serialize(const std::string& strFilename) {
		VCE_TRACE("Serializing {}", strFilename.c_str());
		std::ofstream xOut(strFilename.c_str());

		for (auto [xGuid, xEntity] : m_xEntityMap) {
			xOut << "Entity\n";
			xEntity.Serialize(xOut);
			xOut << "EndEntity\n";
		}
		xOut.close();
	}

	Entity Scene::GetEntityByGuid(GUID xGuid) {
		return m_xEntityMap.at(xGuid.m_uGuid);
	}
	Entity Scene::GetEntityByGuid(GuidType uGuid) {
		return m_xEntityMap.at(uGuid);
	}
}