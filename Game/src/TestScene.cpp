#include "vcepch.h"
#include "TestScene.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
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

		Physics::ResetPhysics();

		std::string strName;
		Material* pxMaterial;

		strName = "sphereSmooth.obj";
		pxMaterial = app->m_xMaterialMap.at("rock2k");
		TransformComponent& xPlayerTrans = m_xPlayerEntity.GetComponent<TransformComponent>();
		xPlayerTrans.SetPosition({ 5,50,5 });
		xPlayerTrans.SetScale({ 10,10,10 });
		ModelComponent& xPlayerModel = m_xPlayerEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		BoxColliderComponent& xPlayerCollider = m_xPlayerEntity.AddComponent<BoxColliderComponent>();

		strName = "sphereSmooth.obj";
		pxMaterial = app->m_xMaterialMap.at("rock2k");
		TransformComponent& xSphereTrans = m_xSphereEntity.GetComponent<TransformComponent>();
		xSphereTrans.SetPosition({ 25,5,25 });
		xSphereTrans.SetScale({ 10,10,10 });
		ModelComponent& xSphereModel = m_xSphereEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		SphereColliderComponent& xSphereCollider = m_xSphereEntity.AddComponent<SphereColliderComponent>();


		strName = "plane.obj";
		pxMaterial = app->m_xMaterialMap.at("crystal2k");
		TransformComponent& xPlaneTrans = m_xPlaneEntity.GetComponent<TransformComponent>();
		xPlaneTrans.SetPosition({ 1,1,1 });
		xPlaneTrans.SetScale({ 1000,1,1000 });
		ModelComponent& xPlaneModel = m_xPlaneEntity.AddComponent<ModelComponent>(strName, pxMaterial);
		BoxColliderComponent& xPlaneCollider = m_xPlaneEntity.AddComponent<BoxColliderComponent>();

		xPlaneCollider.GetRigidBody()->setType(reactphysics3d::BodyType::STATIC);


		m_xGameCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
		m_xEditorCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));

		VCE_TRACE("Physics world num rigidbodies: {}", Physics::s_pxPhysicsWorld->getNbRigidBodies());


		for(ColliderComponent* pxCol : GetAllColliderComponents()){
			pxCol->OnUpdate();
		}
	}

	void TestScene::Reset() {
		bool a = false;
	}
}