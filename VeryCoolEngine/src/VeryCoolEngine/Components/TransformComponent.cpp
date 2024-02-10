#include "vcepch.h"
#include "TransformComponent.h"
#include "reactphysics3d/reactphysics3d.h"

namespace VeryCoolEngine {

	TransformComponent::TransformComponent() {
	}

	TransformComponent::~TransformComponent() {
	}

	void TransformComponent::SetPosition(const glm::vec3& xPos) {
		m_xTransform.SetPosition(xPos);
	}

	void TransformComponent::SetRotation(const glm::quat& xRot) {
		m_xTransform.SetRotationQuat(xRot);
		m_xTransform.UpdateRotation();
	}

	void TransformComponent::SetScale(const glm::vec3& xScale) {
		m_xTransform.SetScale(xScale);
	}
}