#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Physics/Physics.h"

namespace VeryCoolEngine {

	class BoxColliderComponent
	{
	public:
		BoxColliderComponent() = delete;
		BoxColliderComponent(TransformComponent& xTrans);
	private:
		reactphysics3d::RigidBody* m_pxRigidBody;
		reactphysics3d::Collider* m_pxCollider;

		TransformComponent& m_xTransRef;
	};

	class SphereColliderComponent
	{
	public:
		SphereColliderComponent() = delete;
		SphereColliderComponent(TransformComponent& xTrans);
	private:
		reactphysics3d::RigidBody* m_pxRigidBody;
		reactphysics3d::Collider* m_pxCollider;

		TransformComponent& m_xTransRef;
	};

}