#pragma once
#include "reactphysics3d/reactphysics3d.h"
#include "glm/glm.hpp"

namespace VeryCoolEngine {
	class VCEModel;
	class Camera;
	namespace Physics
	{

		enum class CollisionVolumeTYpe : uint32_t {
			AABB,
			OBB,
			Sphere,
			Capsule
		};

		extern reactphysics3d::PhysicsCommon s_xPhysicsCommon;
		extern reactphysics3d::PhysicsWorld* s_pxPhysicsWorld;


		void InitPhysics();
		void UpdatePhysics();

		void AddBoxCollisionVolumeToModel(VCEModel* pxModel, glm::vec3 xHalfExtents);
		void AddSphereCollisionVolumeToModel(VCEModel* pxModel, float fRadius);
		void AddCapsuleCollisionVolumeToModel(VCEModel* pxModel, float fRadius, float fHeight);

		reactphysics3d::Ray BuildRayFromMouse(Camera* pxCam);

		extern double s_fTimestepAccumulator;
		//#TO_TODO: make this a define
		constexpr double s_fDesiredFramerate = 1./60.;
	};

}