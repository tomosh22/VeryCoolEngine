#include "vcepch.h"
#include "TransformComponent.h"
#include "reactphysics3d/reactphysics3d.h"

namespace VeryCoolEngine {

	TransformComponent::TransformComponent() {
		m_pxPhysicsTransform = new reactphysics3d::Transform;
		m_pxPhysicsTransform->setPosition({0,0,0});
		m_pxPhysicsTransform->setOrientation({1,0,0,0});
	}

	TransformComponent::~TransformComponent() {
		delete m_pxPhysicsTransform;
	}

	void TransformComponent::SetPosition(const glm::vec3& xPos) {
		m_xTransform.SetPosition(xPos);
		m_pxPhysicsTransform->setPosition({ xPos.x, xPos.y, xPos.z });
	}

	void TransformComponent::SetRotation(const glm::quat& xRot) {
		m_xTransform.SetRotationQuat(xRot);
		m_xTransform.UpdateRotation();
		m_pxPhysicsTransform->setOrientation({ xRot.x, xRot.y, xRot.z, xRot.w });
	}

	void TransformComponent::SetScale(const glm::vec3& xScale) {
		m_xTransform.SetScale(xScale);
	}
}