#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Physics/Physics.h"
#include "VeryCoolEngine/Scene/Entity.h"

namespace VeryCoolEngine {

	class ColliderComponent {
	public:
		reactphysics3d::RigidBody* GetRigidBody() { return m_pxRigidBody; }
		const EntityID& GetEntityID() const { return m_xEntityID; }
	protected:
		ColliderComponent() = delete;
		ColliderComponent(TransformComponent& xTrans, EntityID xEntityID);
		reactphysics3d::RigidBody* m_pxRigidBody;
		reactphysics3d::Collider* m_pxCollider;

		TransformComponent& m_xTransRef;
		EntityID m_xEntityID;

	};
	class BoxColliderComponent : public ColliderComponent
	{
	public:
		BoxColliderComponent() = delete;
		BoxColliderComponent(TransformComponent& xTrans, EntityID xEntityID);
	};

	class SphereColliderComponent : public ColliderComponent
	{
	public:
		SphereColliderComponent() = delete;
		SphereColliderComponent(TransformComponent& xTrans, EntityID xEntityID);
	};

}