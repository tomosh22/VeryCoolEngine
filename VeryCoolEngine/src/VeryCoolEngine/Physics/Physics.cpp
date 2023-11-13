#include "vcepch.h"
#include "Physics.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {
	physx::PxScene* Physics::s_pxScene;
	physx::PxPhysics* Physics::s_pxPhysics;
	physx::PxPvd* Physics::s_pxPvd;
	physx::PxFoundation* Physics::s_pxFoundation;
	physx::PxDefaultAllocator  Physics::s_xAllocator;
	physx::PxDefaultErrorCallback  Physics::s_xErrorCallback;
	physx::PxDefaultCpuDispatcher* Physics::s_pxDispatcher;
	physx::PxMaterial* Physics::s_pxMaterial;

	void Physics::InitPhysics()
	{
		s_pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_xAllocator, s_xErrorCallback);

		s_pxPvd = PxCreatePvd(*s_pxFoundation);
		physx::PxPvdTransport* pxTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		s_pxPvd->connect(*pxTransport, physx::PxPvdInstrumentationFlag::eALL);

		s_pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_pxFoundation, physx::PxTolerancesScale(), true, s_pxPvd);

		physx::PxSceneDesc xSceneDesc(s_pxPhysics->getTolerancesScale());
		xSceneDesc.gravity = physx::PxVec3(0.0f, -0.5f, 0.0f);
		s_pxDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		xSceneDesc.cpuDispatcher = s_pxDispatcher;
		xSceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		s_pxScene = s_pxPhysics->createScene(xSceneDesc);

		physx::PxPvdSceneClient* pxPvdClient = s_pxScene->getScenePvdClient();
		if (pxPvdClient)
		{
			pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
		s_pxMaterial = s_pxPhysics->createMaterial(0.9f, 0.9f, 0.1f);

		physx::PxRigidStatic* pxGroundPlane = physx::PxCreatePlane(*s_pxPhysics, physx::PxPlane(0, 1, 0, -10), *s_pxMaterial);
		s_pxScene->addActor(*pxGroundPlane);

		
	}

	void Physics::UpdatePhysics() {
		s_pxScene->simulate(Application::GetInstance()->m_fDeltaTime / 1000.f);//#todo dt
		//while (!s_pxScene->checkResults()) { Sleep(1); }
		while (!s_pxScene->fetchResults(true))
			Sleep(1);
	}
}