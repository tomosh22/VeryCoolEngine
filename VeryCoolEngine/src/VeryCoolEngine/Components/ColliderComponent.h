#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Physics/Physics.h"
#include "VeryCoolEngine/Scene/Entity.h"

namespace VeryCoolEngine {

	class ColliderComponent {
	public:
		ColliderComponent() = delete;
		ColliderComponent(TransformComponent& xTrans, Entity* xEntity);
		void Serialize(std::ofstream& xOut);
		reactphysics3d::RigidBody* GetRigidBody() { return m_pxRigidBody; }
		const EntityID& GetEntityID() const { return m_xParentEntity.GetEntityID(); }

		void AddCollider(Physics::CollisionVolumeType eType);
	private:
		reactphysics3d::RigidBody* m_pxRigidBody;
		reactphysics3d::Collider* m_pxCollider;

		Physics::CollisionVolumeType m_eType;

		TransformComponent& m_xTransRef;
		Entity& m_xParentEntity;

	};

}