#include "vcepch.h"
#include "TestScene.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ScriptComponent.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	TestScene::TestScene() :
		m_xPlayerEntity(this),
		m_xPlaneEntity(this),
		m_xSphereEntity(this)
	{
		Application* app = Application::GetInstance();

		//#TO_TODO: will cause problems if i have more than one scene loaded at a time
		app->m_pxCurrentScene = this;


		std::string strName;
		Material* pxMaterial;

		strName = "sphereSmooth.obj";
		pxMaterial = app->m_xMaterialMap.at("rock2k");
		ModelComponent& xPlayerModel = m_xPlayerEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		ScriptComponent& xPlayerScript = m_xPlayerEntity.AddComponent<ScriptComponent>();
		xPlayerScript.SetBehaviour<TestScriptBehaviour>();
		xPlayerScript.Instantiate(xPlayerScript.m_pxScriptBehaviour, &xPlayerScript);
		dynamic_cast<TestScriptBehaviour*>(xPlayerScript.m_pxScriptBehaviour)->m_pxGroundPlane = &m_xPlaneEntity;

		strName = "sphereSmooth.obj";
		pxMaterial = app->m_xMaterialMap.at("rock2k");
		ModelComponent& xSphereModel = m_xSphereEntity.AddComponent<ModelComponent>(strName, pxMaterial);


		strName = "plane.obj";
		pxMaterial = app->m_xMaterialMap.at("crystal2k");
		ModelComponent& xPlaneModel = m_xPlaneEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		

		app->_pRenderer->InitialiseAssets();
		
		m_xEditorCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));

		Reset();

		
	}

	void TestScene::Reset() {
		Application* app = Application::GetInstance();

		Physics::ResetPhysics();

		TransformComponent& xPlayerTrans = m_xPlayerEntity.GetComponent<TransformComponent>();
		xPlayerTrans.SetPosition({ 5,50,5 });
		xPlayerTrans.SetScale({ 10,10,10 });
		BoxColliderComponent& xPlayerCollider = m_xPlayerEntity.AddOrReplaceComponent<BoxColliderComponent>();
		ScriptComponent& xPlayerScript = m_xPlayerEntity.GetComponent<ScriptComponent>();
		xPlayerScript.OnCreate(xPlayerScript.m_pxScriptBehaviour);
		

		
		TransformComponent& xSphereTrans = m_xSphereEntity.GetComponent<TransformComponent>();
		xSphereTrans.SetPosition({ 25,5,25 });
		xSphereTrans.SetScale({ 10,10,10 });
		SphereColliderComponent& xSphereCollider = m_xSphereEntity.AddOrReplaceComponent<SphereColliderComponent>();
		


		
		TransformComponent& xPlaneTrans = m_xPlaneEntity.GetComponent<TransformComponent>();
		xPlaneTrans.SetPosition({ 1,1,1 });
		xPlaneTrans.SetScale({ 1000,1,1000 });
		BoxColliderComponent& xPlaneCollider = m_xPlaneEntity.AddOrReplaceComponent<BoxColliderComponent>();
		xPlaneCollider.GetRigidBody()->setType(reactphysics3d::BodyType::STATIC);

		

		m_xGameCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));


	}
}