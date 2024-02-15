#include "vcepch.h"
#include "TestScene.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ScriptComponent.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	TestScene::TestScene()
	{
		Application* app = Application::GetInstance();

		//#TO_TODO: will cause problems if i have more than one scene loaded at a time
		app->m_pxCurrentScene = this;


		std::string strName;
		Material* pxMaterial;

		Entity xPlayerEntity(this);
		strName = "sphereSmooth.obj";
		pxMaterial = app->m_xMaterialMap.at("rock2k");
		ModelComponent& xPlayerModel = xPlayerEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		ScriptComponent& xPlayerScript = xPlayerEntity.AddComponent<ScriptComponent>();
		xPlayerScript.SetBehaviour<TestScriptBehaviour>();
		xPlayerScript.Instantiate(&xPlayerScript);
		m_xPlayerGuid = xPlayerEntity.GetGuid();
		

		strName = "sphereSmooth.obj";
		pxMaterial = app->m_xMaterialMap.at("rock2k");
		Entity xSphereEntity(this);
		ModelComponent& xSphereModel = xSphereEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		m_xSphereGuid = xSphereEntity.GetGuid();


		strName = "plane.obj";
		pxMaterial = app->m_xMaterialMap.at("crystal2k");
		Entity xPlaneEntity(this);
		ModelComponent& xPlaneModel = xPlaneEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		m_xPlaneGuid = xPlaneEntity.GetGuid();
		dynamic_cast<TestScriptBehaviour*>(xPlayerScript.m_pxScriptBehaviour)->m_xGroundPlaneGuid = xPlaneEntity.GetGuid();

		app->_pRenderer->InitialiseAssets();
		
		m_xEditorCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));

		Reset();

		
	}

	void TestScene::Reset() {
		Application* app = Application::GetInstance();

		Physics::ResetPhysics();

		Entity xPlayerEntity = GetEntityByGuid(m_xPlayerGuid);
		TransformComponent& xPlayerTrans = xPlayerEntity.GetComponent<TransformComponent>();
		xPlayerTrans.SetPosition({ 5,50,5 });
		xPlayerTrans.SetScale({ 10,10,10 });
		ColliderComponent& xPlayerCollider = xPlayerEntity.AddOrReplaceComponent<ColliderComponent>();
		xPlayerCollider.AddCollider(Physics::CollisionVolumeType::OBB);
		ScriptComponent& xPlayerScript = xPlayerEntity.GetComponent<ScriptComponent>();
		xPlayerScript.OnCreate();
		

		Entity xSphereEntity = GetEntityByGuid(m_xSphereGuid);
		TransformComponent& xSphereTrans = xSphereEntity.GetComponent<TransformComponent>();
		xSphereTrans.SetPosition({ 25,5,25 });
		xSphereTrans.SetScale({ 10,10,10 });
		ColliderComponent& xSphereCollider = xSphereEntity.AddOrReplaceComponent<ColliderComponent>();
		xSphereCollider.AddCollider(Physics::CollisionVolumeType::Sphere);
		


		Entity xPlaneEntity = GetEntityByGuid(m_xPlaneGuid);
		TransformComponent& xPlaneTrans = xPlaneEntity.GetComponent<TransformComponent>();
		xPlaneTrans.SetPosition({ 1,1,1 });
		xPlaneTrans.SetScale({ 1000,1,1000 });
		ColliderComponent& xPlaneCollider = xPlaneEntity.AddOrReplaceComponent<ColliderComponent>();
		xPlaneCollider.AddCollider(Physics::CollisionVolumeType::OBB);
		xPlaneCollider.GetRigidBody()->setType(reactphysics3d::BodyType::STATIC);

		

		m_xGameCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));


	}
}