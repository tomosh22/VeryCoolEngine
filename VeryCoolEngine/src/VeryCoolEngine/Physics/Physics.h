#pragma once
#include "reactphysics3d/reactphysics3d.h"

namespace VeryCoolEngine {
	class Physics
	{
	public:

		static reactphysics3d::PhysicsCommon s_xPhysicsCommon;
		static reactphysics3d::PhysicsWorld* s_pxPhysicsWorld;


		static void InitPhysics();
		static void UpdatePhysics();
	};

}