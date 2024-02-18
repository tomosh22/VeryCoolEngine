#include "vcepch.h"
#include "Game.h"
#include <imgui.h>



#include "VeryCoolEngine.h"

#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>

#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ModelComponent.h"



namespace VeryCoolEngine {
	
	Game::Game() {

		//AddModel("ogre.fbx", Transform({ 0,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);

		//AddModel("otherFish.fbx", Transform({ 20,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);


		//AddModel("barrel.fbx", Transform({ 0,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);
		
		_lights.push_back({
				50,200,50,100,
				0,1,0,1
		});

		_lights.push_back({
				50,110,50,50,
				1,1,1,1
			});

		_renderThreadCanStart = true;
		
		return;
	}

	Game::~Game() {}

	


	//declared in Application.h, defined by game
	void Application::GameLoop(float fDt) {
		Game* game = (Game*)Application::GetInstance();

		TransformComponent& xPlayerTrans = m_pxCurrentScene->GetEntityByGuid(m_pxCurrentScene->m_xPlayerGuid).GetComponent<TransformComponent>();
		ColliderComponent& xPlayerPhysics = m_pxCurrentScene->GetEntityByGuid(m_pxCurrentScene->m_xPlayerGuid).GetComponent<ColliderComponent>();

		
		glm::vec3 xCamPos = { xPlayerPhysics.GetRigidBody()->getTransform().getPosition().x,xPlayerPhysics.GetRigidBody()->getTransform().getPosition().y + 15, xPlayerPhysics.GetRigidBody()->getTransform().getPosition().z };
		m_pxCurrentScene->m_xGameCamera.SetPosition(xCamPos);
		m_pxCurrentScene->m_xGameCamera.UpdateRotation();

		if (Input::WasKeyPressed(VCE_KEY_R)) {
			Entity xEntity(m_pxCurrentScene, "Sphere");
			TransformComponent& xTrans = xEntity.GetComponent<TransformComponent>();
			xTrans.SetPosition({rand() / float(RAND_MAX) * 500,20,rand() / float(RAND_MAX) * 500 });
			xTrans.SetScale({ 20,20,20 });

			xEntity.AddComponent<ModelComponent>(GUID(1120456549), GUID(2957329679));
		}
	}

	void Application::CollisionCallback(Entity* pxEntity1, Entity* pxEntity2, Physics::CollisionEventType eType) {
		Game* pxGame = (Game*)Application::GetInstance();
#if 0
		if ((pxModel1 == pxGame->m_pxPlayerModel && pxModel2 == pxGame->m_pxGroundPlane) || (pxModel2 == pxGame->m_pxPlayerModel && pxModel1 == pxGame->m_pxGroundPlane)) {
			if(eType == Physics::CollisionEventType::Start)
				pxGame->m_bPlayerIsOnFloor = true;
			else if (eType == Physics::CollisionEventType::Exit)
				pxGame->m_bPlayerIsOnFloor = false;
		}
#endif
			
	}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


