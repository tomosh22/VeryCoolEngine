#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
#include "VeryCoolEngine/Physics/Physics.h"
#include "ColliderComponent.h"
namespace VeryCoolEngine {

	class ScriptBehaviour {
	public:
		virtual ~ScriptBehaviour() {}
		virtual void OnCreate() = 0;
		virtual void OnUpdate(float fDt) = 0;
		virtual void OnCollision(Entity xOther, Physics::CollisionEventType eCollisionType) = 0;
		virtual std::string GetBehaviourType() = 0;
		std::vector<GUID> m_axGuidRefs;

	};
	class ScriptComponent
	{
	public:

		ScriptComponent(TransformComponent& xTrans, Entity* xEntity) : m_xParentEntity(*xEntity) {};
		~ScriptComponent() {
			delete m_pxScriptBehaviour;
		}
		void Serialize(std::ofstream& xOut);

		void(*Instantiate)(ScriptComponent*);

		ScriptBehaviour* m_pxScriptBehaviour = nullptr;

		Entity m_xParentEntity;

		void OnCreate() { m_pxScriptBehaviour->OnCreate(); }
		void OnUpdate(float fDt) { m_pxScriptBehaviour->OnUpdate(fDt); }
		void OnCollision(Entity xOther, Physics::CollisionEventType eCollisionType) { m_pxScriptBehaviour->OnCollision(xOther, eCollisionType); }

		template<typename T>
		void SetBehaviour() {
			Instantiate = [](ScriptComponent* pxScriptComponent) {pxScriptComponent->m_pxScriptBehaviour = new T(pxScriptComponent); };
		}
	};

	class PlayerController : public ScriptBehaviour {
	public:
		PlayerController(ScriptComponent* pxScriptComponent) : m_xScriptComponent(*pxScriptComponent) {}
		~PlayerController() override {
			
		}

		enum GuidRefernceIndices {
			GroundPlane
		};
		ScriptComponent& m_xScriptComponent;
		bool m_bIsOnGround;
		virtual void OnCreate() override {
			m_bIsOnGround = false;
		}
		virtual void OnUpdate(float fDt) override {
			VCE_TRACE("On ground: {}", m_bIsOnGround);

			constexpr const float fMoveSpeed = 20;
			if (m_bIsOnGround) {
				Scene* pxCurrentScene = m_xScriptComponent.m_xParentEntity.m_pxParentScene;
				ColliderComponent& xPlayerPhysics = m_xScriptComponent.m_xParentEntity.GetComponent<ColliderComponent>();
				reactphysics3d::Vector3 xFinalVelocity(0, 0, 0);
				if (Input::IsKeyPressed(VCE_KEY_SPACE)) {
					xFinalVelocity += reactphysics3d::Vector3(0, 10, 0);
				}
				if (Input::IsKeyPressed(VCE_KEY_W)) {
					glm::mat4 rotation = glm::rotate(pxCurrentScene->m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
					glm::vec4 result = rotation * glm::vec4(0, 0, -1, 1);
					glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
					xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
				}
				if (Input::IsKeyPressed(VCE_KEY_S)) {
					glm::mat4 rotation = glm::rotate(pxCurrentScene->m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
					glm::vec4 result = rotation * glm::vec4(0, 0, -1, 1);
					result *= -1;
					glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
					xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
				}
				if (Input::IsKeyPressed(VCE_KEY_A)) {
					glm::mat4 rotation = glm::rotate(pxCurrentScene->m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
					glm::vec4 result = rotation * glm::vec4(-1, 0, 0, 1);
					glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
					xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
				}
				if (Input::IsKeyPressed(VCE_KEY_D)) {
					glm::mat4 rotation = glm::rotate(pxCurrentScene->m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
					glm::vec4 result = rotation * glm::vec4(-1, 0, 0, 1);
					result *= -1;
					glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
					xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
				}
				xPlayerPhysics.GetRigidBody()->setLinearVelocity(xFinalVelocity);
			}
		}
		virtual void OnCollision(Entity xOther, Physics::CollisionEventType eCollisionType) override {
			if(xOther.GetGuid().m_uGuid == m_axGuidRefs[GroundPlane].m_uGuid) {
				if (eCollisionType == Physics::CollisionEventType::Exit)
					m_bIsOnGround = false;
				else if (eCollisionType == Physics::CollisionEventType::Start || eCollisionType == Physics::CollisionEventType::Stay)
					m_bIsOnGround = true;
			}
		}
		virtual std::string GetBehaviourType() override { return "PlayerController"; }
	};
	
}


