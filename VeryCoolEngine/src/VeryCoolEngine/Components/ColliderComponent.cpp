#include "vcepch.h"
#include "ColliderComponent.h"
#include "ModelComponent.h"

namespace VeryCoolEngine {

	ColliderComponent::ColliderComponent(TransformComponent& xTrans, Entity* xEntity) : m_xTransRef(xTrans), m_xParentEntity(*xEntity) {}

	void ColliderComponent::OnUpdate() {
		TransformComponent& xTrans = m_xParentEntity.GetComponent<TransformComponent>();
		xTrans.m_xTransform.SetPosition({ m_pxRigidBody->getTransform().getPosition().x, m_pxRigidBody->getTransform().getPosition().y, m_pxRigidBody->getTransform().getPosition().z });
		xTrans.m_xTransform.SetRotationQuat({ m_pxRigidBody->getTransform().getOrientation().x, m_pxRigidBody->getTransform().getOrientation().y, m_pxRigidBody->getTransform().getOrientation().z, m_pxRigidBody->getTransform().getOrientation().w });

		if (m_xParentEntity.HasComponent<ModelComponent>()) {
			ModelComponent& xModel = m_xParentEntity.GetComponent<ModelComponent>();
			xModel.GetModel()->m_pxTransform = const_cast<reactphysics3d::Transform*>(&m_pxRigidBody->getTransform());
		}
	}

	BoxColliderComponent::BoxColliderComponent(TransformComponent& xTrans, Entity* xEntity) : ColliderComponent(xTrans, xEntity) {
		reactphysics3d::Transform xPhysTrans;
		xPhysTrans.setPosition({ xTrans.m_xTransform.m_xPosition.x , xTrans.m_xTransform.m_xPosition.y , xTrans.m_xTransform.m_xPosition.z });
		xPhysTrans.setOrientation({ xTrans.m_xTransform.m_xRotationQuat.x , xTrans.m_xTransform.m_xRotationQuat.y , xTrans.m_xTransform.m_xRotationQuat.z, xTrans.m_xTransform.m_xRotationQuat.w });
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(xPhysTrans);

		reactphysics3d::BoxShape* pxShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(xTrans.m_xTransform.m_xScale.x, xTrans.m_xTransform.m_xScale.y, xTrans.m_xTransform.m_xScale.z));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	}


	SphereColliderComponent::SphereColliderComponent(TransformComponent& xTrans, Entity* xEntity) : ColliderComponent(xTrans, xEntity) {
		reactphysics3d::Transform xPhysTrans;
		xPhysTrans.setPosition({ xTrans.m_xTransform.m_xPosition.x , xTrans.m_xTransform.m_xPosition.y , xTrans.m_xTransform.m_xPosition.z });
		xPhysTrans.setOrientation({ xTrans.m_xTransform.m_xRotationQuat.x , xTrans.m_xTransform.m_xRotationQuat.y , xTrans.m_xTransform.m_xRotationQuat.z, xTrans.m_xTransform.m_xRotationQuat.w });
		m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(xPhysTrans);

		reactphysics3d::SphereShape* pxShape = Physics::s_xPhysicsCommon.createSphereShape(glm::length(xTrans.m_xTransform.m_xScale));

		m_pxCollider = m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());

		m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	}
}