#include "vcepch.h"
#include "ColliderComponent.h"
#include "ModelComponent.h"

namespace VeryCoolEngine {

	ColliderComponent::ColliderComponent(TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity) {}


	BoxColliderComponent::BoxColliderComponent(TransformComponent& xTrans, Entity* xEntity) : ColliderComponent(xTrans, xEntity) {
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*xTrans.GetTransform_Unsafe());

		reactphysics3d::BoxShape* pxShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(xTrans.m_xScale.x, xTrans.m_xScale.y, xTrans.m_xScale.z));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);

		xTrans.m_pxRigidBody = m_pxRigidBody;
	}


	SphereColliderComponent::SphereColliderComponent(TransformComponent& xTrans, Entity* xEntity) : ColliderComponent(xTrans, xEntity) {
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*xTrans.GetTransform_Unsafe());

		reactphysics3d::SphereShape* pxShape = Physics::s_xPhysicsCommon.createSphereShape(glm::length(xTrans.m_xScale));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);

		xTrans.m_pxRigidBody = m_pxRigidBody;
	}
}