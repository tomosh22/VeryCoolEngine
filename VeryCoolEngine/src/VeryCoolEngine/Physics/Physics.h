#pragma once

#include "PxPhysicsAPI.h"
#include "snippets/snippetcustomgeometry/VoxelMap.h"

namespace VeryCoolEngine {
	class Physics
	{
	public:
		static physx::PxScene* s_pxScene;
		static physx::PxPhysics* s_pxPhysics;
		static physx::PxPvd* s_pxPvd;
		static physx::PxFoundation* s_pxFoundation;
		static physx::PxDefaultAllocator s_xAllocator;
		static physx::PxDefaultErrorCallback s_xErrorCallback;
		static physx::PxDefaultCpuDispatcher* s_pxDispatcher;
		static physx::PxMaterial* s_pxMaterial;

		static void InitPhysics();
		static void UpdatePhysics();
	};

}

