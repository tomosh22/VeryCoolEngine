#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace VeryCoolEngine {
	class Transform
	{
	public:

		static std::vector<glm::quat> uniqueQuats; //TEST
		Transform() {};
		Transform(glm::vec3 xPos, glm::vec3 xScale, glm::quat xRot = glm::quat_identity<float, glm::packed_highp>()) : m_xPosition(xPos), m_xScale(xScale), m_xRotationQuat(xRot) {}

		void SetPosition(const glm::vec3& pos);
		void SetRotationQuat(const glm::quat& rot);
		inline void UpdateMatrix();
		void UpdateRotation();
		glm::mat4 m_xMatrix;

		glm::quat m_xRotationQuat = glm::quat_identity<float, glm::packed_highp>();
		float m_fRoll = 0, m_fYaw = 0, m_fPitch = 0;
		glm::vec3 m_xPosition = { 0,0,0 };
		glm::vec3 m_xScale = { 1,1,1 };

		static glm::mat4 RotationMatFromQuat(const glm::quat& quat);
		static glm::mat4 RotationMatFromVec3(float degrees, const glm::vec3& axis);
		static glm::quat EulerAnglesToQuat(float roll, float yaw, float pitch);
	private:
		float m_fPrevRoll = 0, m_fPrevYaw = 0, m_fPrevPitch = 0;
	};

}

