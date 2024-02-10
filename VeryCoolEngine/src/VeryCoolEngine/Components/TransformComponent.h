#pragma once
#include "VeryCoolEngine/Transform.h"

namespace reactphysics3d {
	class Transform;
}
namespace VeryCoolEngine {

	class TransformComponent
	{
	public:
		TransformComponent();
		~TransformComponent();
		void SetPosition(const glm::vec3& xPos);
		void SetRotation(const glm::quat& xRot);
		void SetScale(const glm::vec3& xScale);
		//#TO_TODO: this is just a proxy right now
		Transform m_xTransform;

	};

}