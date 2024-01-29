#pragma once
#include "reactphysics3d/reactphysics3d.h"

namespace VeryCoolEngine {
	class Camera;
	class Physics
	{
	public:

		enum class CollisionVolumeTYpe : uint32_t {
			AABB,
			OBB,
			Sphere,
			Capsule
		};

		static reactphysics3d::PhysicsCommon s_xPhysicsCommon;
		static reactphysics3d::PhysicsWorld* s_pxPhysicsWorld;


		static void InitPhysics();
		static void UpdatePhysics();

		static reactphysics3d::Ray BuildRayFromMouse(Camera* pxCam);

		static double s_fTimestepAccumulator;
		//#TO_TODO: make this a define
		static constexpr double s_fDesiredFramerate = 1./60.;
	};

}