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

					VCEModel* pxModel1 = reinterpret_cast<VCEModel*>(xContactPair.getBody1()->getUserData());
					VCEModel* pxModel2 = reinterpret_cast<VCEModel*>(xContactPair.getBody2()->getUserData());
					if ((pxModel1 == m_pxGame->m_pxPlayerModel && pxModel2 == m_pxGame->m_pxGroundPlane) || (pxModel2 == m_pxGame->m_pxPlayerModel && pxModel1 == m_pxGame->m_pxGroundPlane)) {
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

		

		VCEModel* m_pxPlayerModel;
		VCEModel* m_pxGroundPlane;

		bool m_bPlayerIsOnFloor = false;
	};


}
