#include "vcepch.h"
#include "ColliderComponent.h"
#include "ModelComponent.h"

namespace VeryCoolEngine {

	ColliderComponent::ColliderComponent(TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity) {
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*xTrans.GetTransform_Unsafe());

		reactphysics3d::BoxShape* pxShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(xTrans.m_xScale.x, xTrans.m_xScale.y, xTrans.m_xScale.z));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		
		m_pxRigidBody->setUserData(reinterpret_cast<void*>(xEntity->GetGuid().m_uGuid));

		xTrans.m_pxRigidBody = m_pxRigidBody;
	}

	void ColliderComponent::AddCollider(Physics::CollisionVolumeType eVolumeType, Physics::RigidBodyType eRigidBodyType) {
		m_eVolumeType = eVolumeType;
		m_eRigidBodyType = eRigidBodyType;
		switch (eVolumeType) {
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

		switch (eRigidBodyType) {
		case Physics::RigidBodyType::Dynamic:
			m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
			break;
		case Physics::RigidBodyType::Static:
			m_pxRigidBody->setType(reactphysics3d::BodyType::STATIC);
			break;
		}
	}

	void ColliderComponent::Serialize(std::ofstream& xOut) {
		xOut << "ColliderComponent\n";
		switch (m_eVolumeType) {
		case Physics::CollisionVolumeType::OBB:
			xOut << "OBB\n";
			break;
		case Physics::CollisionVolumeType::Sphere:
			xOut << "Sphere\n";
			break;
		}
		switch (m_eRigidBodyType) {
		case Physics::RigidBodyType::Dynamic:
			xOut << "Dynamic\n";
			break;
		case Physics::RigidBodyType::Static:
			xOut << "Static\n";
			break;
		}
	}
}