#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Physics/Physics.h"
#include "VeryCoolEngine/Scene/Entity.h"

namespace VeryCoolEngine {

	class ColliderComponent {
	public:
		reactphysics3d::RigidBody* GetRigidBody() { return m_pxRigidBody; }
		const EntityID& GetEntityID() const { return m_xParentEntity.GetEntityID(); }

		void OnUpdate();
	protected:
		ColliderComponent() = delete;
		ColliderComponent(TransformComponent& xTrans, Entity* xEntity);
		reactphysics3d::RigidBody* m_pxRigidBody;
		reactphysics3d::Collider* m_pxCollider;

		TransformComponent& m_xTransRef;
		Entity& m_xParentEntity;

	};
	class BoxColliderComponent : public ColliderComponent
	{
	public:
		BoxColliderComponent() = delete;
		BoxColliderComponent(TransformComponent& xTrans, Entity* xEntity);
	};

	class SphereColliderComponent : public ColliderComponent
	{
	public:
		SphereColliderComponent() = delete;
		SphereColliderComponent(TransformComponent& xTrans, Entity* xEntity);
	};

}