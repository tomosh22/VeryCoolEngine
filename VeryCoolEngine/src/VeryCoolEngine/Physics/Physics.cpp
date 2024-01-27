#include "vcepch.h"
#include "Physics.h"
#include "VeryCoolEngine/Application.h"
#include "VeryCoolEngine/Renderer/Camera.h"

namespace VeryCoolEngine {

	reactphysics3d::PhysicsCommon Physics::s_xPhysicsCommon;
	reactphysics3d::PhysicsWorld* Physics::s_pxPhysicsWorld;

	void Physics::InitPhysics()
	{
		s_pxPhysicsWorld = s_xPhysicsCommon.createPhysicsWorld();
		s_pxPhysicsWorld->setGravity({ 0,-1,0 });

	}

	void Physics::UpdatePhysics() {
		s_pxPhysicsWorld->update(Application::GetInstance()->m_fDeltaTime / 1000.f);
	}

	reactphysics3d::Ray Physics::BuildRayFromMouse(Camera* pxCam)
	{
		Application* pxApp = Application::GetInstance();
		Renderer* pxRenderer = pxApp->_pRenderer;

		std::pair<double, double> xCurrentMousePos = pxCam->prevMousePos;

		glm::vec3 xNearPos = { xCurrentMousePos.first, xCurrentMousePos.second, 0.0f };
		glm::vec3 xFarPos = { xCurrentMousePos.first, xCurrentMousePos.second, 1.0f };

		glm::vec3 xOrigin = pxCam->ScreenSpaceToWorldSpace(xNearPos);
		glm::vec3 xDest = pxCam->ScreenSpaceToWorldSpace(xFarPos);

		reactphysics3d::Vector3 xRayOrigin = { xOrigin.x, xOrigin.y, xOrigin.z };
		reactphysics3d::Vector3 xRayDest = { xDest.x, xDest.y, xDest.z };

		reactphysics3d::Ray xRet(xRayOrigin, xRayDest);

		return xRet;
	}

}