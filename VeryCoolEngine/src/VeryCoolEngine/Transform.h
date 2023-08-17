#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace VeryCoolEngine {
	class Transform
	{
	public:

		void SetPosition(const glm::vec3& pos);
		void SetRotationQuat(const glm::quat& rot);
		inline void UpdateMatrix();
		void UpdateRotation();
		glm::mat4 _matrix;

		glm::quat _rotationQuat = glm::quat_identity<float, glm::packed_highp>();
		float _roll = 0, _yaw = 0, _pitch = 0;
		glm::vec3 _position = { 0,0,0 };
		glm::vec3 _scale = { 1,1,1 };

		glm::mat4 RotationMatFromQuat(const glm::quat& quat);
		glm::mat4 RotationMatFromVec3(float degrees, const glm::vec3& axis);
		glm::quat EulerAnglesToQuat(float roll, float yaw, float pitch);
	private:
		float _prevRoll = 0, _prevYaw = 0, _prevPitch = 0;
	};

}

