#include "vcepch.h"
#include "Physics.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Renderer/Camera.h"
#include "VeryCoolEngine/Renderer/Model.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/Components/ScriptComponent.h"

namespace VeryCoolEngine {

	namespace Physics {
		reactphysics3d::PhysicsCommon s_xPhysicsCommon;
		reactphysics3d::PhysicsWorld* s_pxPhysicsWorld = nullptr;
		double s_fTimestepAccumulator = 0;
		PhysicsEventListener s_xEventListener;
	}

	void Physics::InitPhysics()
	{
		s_pxPhysicsWorld = s_xPhysicsCommon.createPhysicsWorld();
		s_pxPhysicsWorld->setGravity({ 0,-9.81,0 });
		s_xEventListener = PhysicsEventListener(Application::GetInstance());
		s_pxPhysicsWorld->setEventListener(&s_xEventListener);
	}

	void Physics::UpdatePhysics() {
		while (s_fTimestepAccumulator > s_fDesiredFramerate) {
			s_pxPhysicsWorld->update(s_fDesiredFramerate);
			s_fTimestepAccumulator -= s_fDesiredFramerate;
		}
	}

	void Physics::ResetPhysics() {
		s_xPhysicsCommon.destroyPhysicsWorld(s_pxPhysicsWorld);
		s_pxPhysicsWorld = s_xPhysicsCommon.createPhysicsWorld();
		s_pxPhysicsWorld->setGravity({ 0,-9.81,0 });
		s_pxPhysicsWorld->setEventListener(&s_xEventListener);
	}

	reactphysics3d::Ray Physics::BuildRayFromMouse(Camera* pxCam)
	{
		Application* pxApp = Application::GetInstance();
		Renderer* pxRenderer = pxApp->_pRenderer;

		std::pair<double, double> xCurrentMousePos = pxCam->prevMousePos;

		double fX = xCurrentMousePos.first;
		double fY = xCurrentMousePos.second;

#ifdef VCE_USE_EDITOR
		//accounting for extra padding from imgui border
		fX -= 10;
		fY -= 45;

		//#TO_TODO: what happens on window resize?
		fX /= (float)VCE_GAME_WIDTH / float(VCE_GAME_WIDTH + VCE_EDITOR_ADDITIONAL_WIDTH);
		fY /= (float)VCE_GAME_HEIGHT / float(VCE_GAME_HEIGHT + VCE_EDITOR_ADDITIONAL_HEIGHT);
#endif

		glm::vec3 xNearPos = { fX, fY, 0.0f };
		glm::vec3 xFarPos = { fX, fY, 1.0f };

		glm::vec3 xOrigin = pxCam->ScreenSpaceToWorldSpace(xNearPos);
		glm::vec3 xDest = pxCam->ScreenSpaceToWorldSpace(xFarPos);

		reactphysics3d::Vector3 xRayOrigin = { xOrigin.x, xOrigin.y, xOrigin.z };
		reactphysics3d::Vector3 xRayDest = { xDest.x, xDest.y, xDest.z };

		reactphysics3d::Ray xRet(xRayOrigin, xRayDest);

		return xRet;
	}


	Physics::PhysicsEventListener::PhysicsEventListener(VeryCoolEngine::Application* pxApp) : m_pxApp(pxApp) {}

	void Physics::PhysicsEventListener::onContact(const CollisionCallback::CallbackData& xCallbackData) {
		Application* pxApp = Application::GetInstance();
		for (uint32_t i = 0; i < xCallbackData.getNbContactPairs(); i++) {
			CollisionCallback::ContactPair xContactPair = xCallbackData.getContactPair(i);

			Entity xEntity1 = pxApp->m_pxCurrentScene->GetEntityByGuid(reinterpret_cast<GuidType>(xContactPair.getBody1()->getUserData()));
			Entity xEntity2 = pxApp->m_pxCurrentScene->GetEntityByGuid(reinterpret_cast<GuidType>(xContactPair.getBody2()->getUserData()));
			switch (xContactPair.getEventType()) {
			case reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStart:
				if (xEntity1.HasComponent<ScriptComponent>()) {
					ScriptComponent& xScript = xEntity1.GetComponent<ScriptComponent>();
					xScript.OnCollision(xEntity2, CollisionEventType::Start);
				}
				if (xEntity2.HasComponent<ScriptComponent>()) {
					ScriptComponent& xScript = xEntity2.GetComponent<ScriptComponent>();
					xScript.OnCollision(xEntity1, CollisionEventType::Start);
				}
				break;
			case reactphysics3d::CollisionCallback::ContactPair::EventType::ContactExit:
				if (xEntity1.HasComponent<ScriptComponent>()) {
					ScriptComponent& xScript = xEntity1.GetComponent<ScriptComponent>();
					xScript.OnCollision( xEntity2, CollisionEventType::Exit);
				}
				if (xEntity2.HasComponent<ScriptComponent>()) {
					ScriptComponent& xScript = xEntity2.GetComponent<ScriptComponent>();
					xScript.OnCollision( xEntity1, CollisionEventType::Exit);
				}
				break;
			case reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStay:
				if (xEntity1.HasComponent<ScriptComponent>()) {
					ScriptComponent& xScript = xEntity1.GetComponent<ScriptComponent>();
					xScript.OnCollision(xEntity2, CollisionEventType::Stay);
				}
				if (xEntity2.HasComponent<ScriptComponent>()) {
					ScriptComponent& xScript = xEntity2.GetComponent<ScriptComponent>();
					xScript.OnCollision(xEntity1, CollisionEventType::Stay);
				}
				break;
			}
		}
	}
}