#include "vcepch.h"
#include "Physics.h"
#include "VeryCoolEngine/Application.h"

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

}