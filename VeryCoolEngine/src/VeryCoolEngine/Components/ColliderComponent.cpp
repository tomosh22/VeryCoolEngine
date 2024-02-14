#include "vcepch.h"
#include "ColliderComponent.h"
#include "ModelComponent.h"

namespace VeryCoolEngine {

	ColliderComponent::ColliderComponent(TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*xTrans.GetTransform_Unsafe());

		reactphysics3d::BoxShape* pxShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(xTrans.m_xScale.x, xTrans.m_xScale.y, xTrans.m_xScale.z));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
		m_pxRigidBody->setUserData(xEntity);

		xTrans.m_pxRigidBody = m_pxRigidBody;
	}

	void ColliderComponent::AddCollider(Physics::CollisionVolumeType eType) {
		switch (eType) {
		case Physics::CollisionVolumeType::OBB:
		{
			reactphysics3d::BoxShape* pxOBBShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(m_xTransRef.m_xScale.x, m_xTransRef.m_xScale.y, m_xTransRef.m_xScale.z));

			m_pxCollider = m_pxRigidBody->addCollider(pxOBBShape, reactphysics3d::Transform::identity());
		}
			break;
		case Physics::CollisionVolumeType::Sphere:
		{
			reactphysics3d::SphereShape* pxSphereShape = Physics::s_xPhysicsCommon.createSphereShape(glm::length(m_xTransRef.m_xScale));

			m_pxCollider = m_pxRigidBody->addCollider(pxSphereShape, reactphysics3d::Transform::identity());
		}
			break;
		}
	}
}