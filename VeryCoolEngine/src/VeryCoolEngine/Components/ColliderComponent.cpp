#include "vcepch.h"
#include "ColliderComponent.h"

namespace VeryCoolEngine {

	ColliderComponent::ColliderComponent(TransformComponent& xTrans, EntityID xEntityID) : m_xTransRef(xTrans), m_xEntityID(xEntityID) {}

	BoxColliderComponent::BoxColliderComponent(TransformComponent& xTrans, EntityID xEntityID) : ColliderComponent(xTrans, xEntityID) {
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*xTrans.m_pxPhysicsTransform);

		reactphysics3d::BoxShape* pxShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(xTrans.m_xTransform.m_xScale.x, xTrans.m_xTransform.m_xScale.y, xTrans.m_xTransform.m_xScale.z));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	}

	SphereColliderComponent::SphereColliderComponent(TransformComponent& xTrans, EntityID xEntityID) : ColliderComponent(xTrans, xEntityID) {
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*xTrans.m_pxPhysicsTransform);

		reactphysics3d::SphereShape* pxShape = Physics::s_xPhysicsCommon.createSphereShape(glm::length(xTrans.m_xTransform.m_xScale));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	}
}