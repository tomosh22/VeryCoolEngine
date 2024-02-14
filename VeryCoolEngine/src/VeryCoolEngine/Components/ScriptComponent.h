#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/Physics/Physics.h"
#include "ColliderComponent.h"
namespace VeryCoolEngine {

	class ScriptBehaviour;
	class ScriptComponent
	{
	public:

		ScriptComponent(TransformComponent& xTrans, Entity* xEntity) : m_xParentEntity(*xEntity) {};

		void(*Instantiate)(ScriptBehaviour*&, ScriptComponent*);
		void(*OnCreate)(ScriptBehaviour*&);
		void(*OnUpdate)(ScriptBehaviour*&, float);
		void(*OnCollision)(ScriptBehaviour*&, Entity*, Physics::CollisionEventType);

		ScriptBehaviour* m_pxScriptBehaviour = nullptr;

		Entity& m_xParentEntity;

		template<typename T>
		void SetBehaviour() {
			Instantiate = [](ScriptBehaviour*& pxScriptBehaviour, ScriptComponent* pxScriptComponent) {pxScriptBehaviour = new T(pxScriptComponent); };
			OnCreate = [](ScriptBehaviour*& pxScriptBehaviour) {dynamic_cast<T*>(pxScriptBehaviour)->OnCreate(); };
			OnUpdate = [](ScriptBehaviour*& pxScriptBehaviour, float fDt) {dynamic_cast<T*>(pxScriptBehaviour)->OnUpdate(fDt); };
			OnCollision = [](ScriptBehaviour*& pxScriptBehaviour, Entity* pxOther, Physics::CollisionEventType eCollisionType){dynamic_cast<T*>(pxScriptBehaviour)->OnCollision(pxOther, eCollisionType); };
		}
	};

	class ScriptBehaviour {
	public:
		virtual void OnCreate() = 0;
		virtual void OnUpdate(float fDt) = 0;
		virtual void OnCollision(Entity* pxOther, Physics::CollisionEventType eCollisionType) = 0;

	};
	class TestScriptBehaviour : public ScriptBehaviour {
	public:
		TestScriptBehaviour(ScriptComponent* pxScriptComponent) : m_xScriptComponent(*pxScriptComponent) {}
		Entity* m_pxGroundPlane = nullptr;
		ScriptComponent& m_xScriptComponent;
		bool m_bIsOnGround;
		virtual void OnCreate() override {
			m_bIsOnGround = false;
		}
		virtual void OnUpdate(float fDt) override {
			VCE_TRACE("On ground: {}", m_bIsOnGround);
		}
		virtual void OnCollision(Entity* pxOther, Physics::CollisionEventType eCollisionType) override {
			if (pxOther == m_pxGroundPlane) {
				if (eCollisionType == Physics::CollisionEventType::Exit)
					m_bIsOnGround = false;
				else if (eCollisionType == Physics::CollisionEventType::Start || eCollisionType == Physics::CollisionEventType::Stay)
					m_bIsOnGround = true;
			}
		}
	};
	
}


