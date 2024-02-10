#include "vcepch.h"
#include "Physics.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Renderer/Camera.h"
#include "VeryCoolEngine/Renderer/Model.h"

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


	void Physics::AddBoxCollisionVolumeToModel(VCEModel* pxModel, glm::vec3 xHalfExtents)
	{
		pxModel->m_bUsePhysics = true;

		pxModel->m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*pxModel->m_pxTransform);

		reactphysics3d::BoxShape* pxShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(xHalfExtents.x, xHalfExtents.y, xHalfExtents.z));
		reactphysics3d::Collider* pxCollider = pxModel->m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());
		pxModel->m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);

		//#TO: so I can access model from within collision callback
		pxModel->m_pxRigidBody->setUserData(pxModel);
	}

	void Physics::AddSphereCollisionVolumeToModel(VCEModel* pxModel, float fRadius)
	{
		pxModel->m_bUsePhysics = true;

		pxModel->m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*pxModel->m_pxTransform);

		reactphysics3d::SphereShape* pxShape = Physics::s_xPhysicsCommon.createSphereShape(fRadius);
		reactphysics3d::Collider* pxCollider = pxModel->m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());
		pxModel->m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);

		//#TO: so I can access model from within collision callback
		pxModel->m_pxRigidBody->setUserData(pxModel);
	}

	void Physics::AddCapsuleCollisionVolumeToModel(VCEModel* pxModel, float fRadius, float fHeight)
	{
		pxModel->m_bUsePhysics = true;

		pxModel->m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*pxModel->m_pxTransform);

		reactphysics3d::CapsuleShape* pxShape = Physics::s_xPhysicsCommon.createCapsuleShape(fRadius, fHeight);
		reactphysics3d::Collider* pxCollider = pxModel->m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());
		pxModel->m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);

		//#TO: so I can access model from within collision callback
		pxModel->m_pxRigidBody->setUserData(pxModel);
	}


	Physics::PhysicsEventListener::PhysicsEventListener(VeryCoolEngine::Application* pxApp) : m_pxApp(pxApp) {}

	void Physics::PhysicsEventListener::onContact(const CollisionCallback::CallbackData& xCallbackData) {
		for (uint32_t i = 0; i < xCallbackData.getNbContactPairs(); i++) {
			CollisionCallback::ContactPair xContactPair = xCallbackData.getContactPair(i);

			VCEModel* pxModel1 = reinterpret_cast<VCEModel*>(xContactPair.getBody1()->getUserData());
			VCEModel* pxModel2 = reinterpret_cast<VCEModel*>(xContactPair.getBody2()->getUserData());
			switch (xContactPair.getEventType()) {
			case reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStart:
				m_pxApp->CollisionCallback(pxModel1, pxModel2, CollisionEventType::Start);
				break;
			case reactphysics3d::CollisionCallback::ContactPair::EventType::ContactExit:
				m_pxApp->CollisionCallback(pxModel1, pxModel2, CollisionEventType::Exit);
				break;
			case reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStay:
				m_pxApp->CollisionCallback(pxModel1, pxModel2, CollisionEventType::Stay);
				break;
			}

		}
	}
}