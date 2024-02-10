#include "vcepch.h"
#include "Scene.h"
#include "VeryCoolEngine/Physics/Physics.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Application.h"
#include "Entity.h"

namespace VeryCoolEngine {
	void Scene::Reset() {
		Application* app = Application::GetInstance();

		Physics::ResetPhysics();

		std::string strName;
		Material* pxMaterial;

		Entity xSphereEntity;
		strName = "sphereSmooth.obj";
		pxMaterial = app->m_xMaterialMap.at("rock2k");
		TransformComponent& xSphereTrans = xSphereEntity.GetComponent<TransformComponent>();
		xSphereTrans.SetPosition({ 5,5,5 });
		xSphereTrans.SetScale({ 10,10,10 });
		ModelComponent& xSphereModel = xSphereEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		SphereColliderComponent& xSphereCollider = xSphereEntity.AddComponent<SphereColliderComponent>();


		Entity xPlaneEntity;
		strName = "plane.obj";
		pxMaterial = app->m_xMaterialMap.at("crystal2k");
		TransformComponent& xPlaneTrans = xPlaneEntity.GetComponent<TransformComponent>();
		xPlaneTrans.SetPosition({ 1,1,1 });
		xPlaneTrans.SetScale({ 1000,1,1000 });
		ModelComponent& xPlaneModel = xPlaneEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		BoxColliderComponent& xPlaneCollider = xPlaneEntity.AddComponent<BoxColliderComponent>();


		m_xGameCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
	}

	std::vector<ColliderComponent*> Scene::GetAllColliderComponents() {
		std::vector<ColliderComponent*> xRet;
		for (BoxColliderComponent* pxCol : GetAllOfComponentType<BoxColliderComponent>())
			xRet.push_back(pxCol);
		for (SphereColliderComponent* pxCol : GetAllOfComponentType<SphereColliderComponent>())
			xRet.push_back(pxCol);
		return xRet;
	}
}