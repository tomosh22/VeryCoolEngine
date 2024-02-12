#include "vcepch.h"
#include "TransformComponent.h"
#include "reactphysics3d/reactphysics3d.h"

namespace VeryCoolEngine {

	TransformComponent::TransformComponent() {
		m_pxTransform = new reactphysics3d::Transform;
	}

	TransformComponent::~TransformComponent() {
	}

	void TransformComponent::SetPosition(const glm::vec3& xPos) {
		m_pxTransform->setPosition({xPos.x, xPos.y, xPos.z});
	}

	void TransformComponent::SetRotation(const glm::quat& xRot) {
		m_pxTransform->setOrientation({ xRot.x, xRot.y, xRot.z, xRot.w });
	}

	void TransformComponent::SetScale(const glm::vec3& xScale) {
		m_xScale = xScale;
	}

	reactphysics3d::Transform* TransformComponent::GetTransform() {
		return m_pxRigidBody ? const_cast<reactphysics3d::Transform*>(&m_pxRigidBody->getTransform()) : m_pxTransform;
	}

	reactphysics3d::Transform* TransformComponent::GetTransform_Unsafe() {
		return m_pxTransform;
	}
}