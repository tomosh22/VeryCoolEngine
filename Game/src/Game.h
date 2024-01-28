#pragma once

#include "VeryCoolEngine/Application.h"
#include <glm/glm.hpp>
#include "VeryCoolEngine/BlockWorld/Block.h"
#include "VeryCoolEngine/BlockWorld/Chunk.h"
#include <map>

#include "VeryCoolEngine/Physics/Physics.h"

namespace VeryCoolEngine {

	class Game : public Application {
	public:

		class PhysicsEventListener : public reactphysics3d::EventListener {
		public:
			PhysicsEventListener() = default;
			PhysicsEventListener(Game* pxGame) : m_pxGame(pxGame) {}
			void onContact(const CollisionCallback::CallbackData& xCallbackData) override {
				for (uint32_t i = 0; i < xCallbackData.getNbContactPairs(); i++) {
					CollisionCallback::ContactPair xContactPair = xCallbackData.getContactPair(i);

					reactphysics3d::RigidBody* pxPlayerBody = m_pxGame->m_pxPlayerModel->m_pxRigidBody;
					reactphysics3d::RigidBody* pxGroundBody = m_pxGame->m_pxGroundPlane->m_pxRigidBody;
					if ((xContactPair.getBody1() == pxPlayerBody && xContactPair.getBody2() == pxGroundBody) || (xContactPair.getBody2() == pxPlayerBody && xContactPair.getBody1() == pxGroundBody)) {
						reactphysics3d::CollisionCallback::ContactPair::EventType eContactType = xContactPair.getEventType();

						if (eContactType == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStart)
							m_pxGame->m_bPlayerIsOnFloor = true;
						else if (eContactType == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactExit)
							m_pxGame->m_bPlayerIsOnFloor = false;

						//don't care about any other collisions (yet)
						break;
					}
				}
			}

			Game* m_pxGame;
		} m_xPhysicsEventListener;

		Game();
		~Game();

		void GenerateChunks();
		void UploadChunks();

		

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Material* pxMaterial, Transform xTrans);

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Transform xTrans);

		void AddBoxCollisionVolumeToModel(VCEModel* pxModel, glm::vec3 xHalfExtents);
		void AddSphereCollisionVolumeToModel(VCEModel* pxModel, float fRadius);
		void AddCapsuleCollisionVolumeToModel(VCEModel* pxModel, float fRadius, float fHeight);
		

		VCEModel* m_pxPlayerModel;
		VCEModel* m_pxGroundPlane;

		bool m_bPlayerIsOnFloor = false;
	};


}
