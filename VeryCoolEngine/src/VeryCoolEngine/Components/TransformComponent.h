#pragma once
#include "VeryCoolEngine/Transform.h"

namespace reactphysics3d {
	class Transform;
	class RigidBody;
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

		reactphysics3d::Transform* GetTransform();
		glm::vec3 m_xScale;
		reactphysics3d::RigidBody* m_pxRigidBody = nullptr;
		
	private:
		friend class BoxColliderComponent;
		friend class SphereColliderComponent;
		reactphysics3d::Transform* GetTransform_Unsafe();
		reactphysics3d::Transform* m_pxTransform;
		
		

	};

}