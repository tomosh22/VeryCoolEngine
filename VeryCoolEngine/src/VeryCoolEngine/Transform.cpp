#include "vcepch.h"
#include "Transform.h"
 
namespace VeryCoolEngine {

	inline void Transform::UpdateMatrix() {
		glm::mat4 trans = glm::translate(glm::identity<glm::mat4>(), _position);
		glm::mat4 rotation = RotationMatFromQuat(_rotationQuat);
		glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), _scale);
		_matrix = trans * rotation * scale;
		//__debugbreak();
	}
	void Transform::UpdateRotation()
	{
		if (_prevRoll != _roll || _prevYaw != _yaw || _prevPitch != _pitch) _rotationQuat = EulerAnglesToQuat(_roll, _yaw, _pitch);
		_prevRoll = _roll;
		_prevYaw = _yaw;
		_prevPitch = _pitch;
	}
	void Transform::SetPosition(const glm::vec3& pos)
	{
		_position = pos;
		UpdateMatrix();
	}

	void Transform::SetRotationQuat(const glm::quat& rot)
	{
		_rotationQuat = rot;
		UpdateMatrix();
	}

	//credit Rich Davison
	glm::mat4 Transform::RotationMatFromQuat(const glm::quat& quat)
	{
		glm::mat4 mat;
		float yy = quat.y * quat.y;
		float zz = quat.z * quat.z;
		float xy = quat.x * quat.y;
		float zw = quat.z * quat.w;
		float xz = quat.x * quat.z;
		float yw = quat.y * quat.w;
		float xx = quat.x * quat.x;
		float yz = quat.y * quat.z;
		float xw = quat.x * quat.w;

		mat[0][0] = 1 - 2 * yy - 2 * zz;
		mat[0][1] = 2 * xy + 2 * zw;
		mat[0][2] = 2 * xz - 2 * yw;

		mat[1][0] = 2 * xy - 2 * zw;
		mat[1][1] = 1 - 2 * xx - 2 * zz;
		mat[1][2] = 2 * yz + 2 * xw;

		mat[2][0] = 2 * xz + 2 * yw;
		mat[2][1] = 2 * yz - 2 * xw;
		mat[2][2] = 1 - 2 * xx - 2 * yy;

		mat[0][3] = 0;
		mat[1][3] = 0;
		mat[2][3] = 0;
		mat[3][3] = 1;

		mat[3][0] = 0;
		mat[3][1] = 0;
		mat[3][2] = 0;
		return mat;
	}
	//credit Rich Davison
	glm::mat4 Transform::RotationMatFromVec3(float degrees, const glm::vec3& axis)
	{
		glm::mat4 mat;

		float c = cos((float)glm::radians(degrees));
		float s = sin((float)glm::radians(degrees));

		

		mat[0][0] = (axis.x * axis.x) * (1.0f - c) + c;
		mat[0][1] = (axis.y * axis.x) * (1.0f - c) + (axis.z * s);
		mat[0][2] = (axis.z * axis.x) * (1.0f - c) - (axis.y * s);
		 
		mat[1][0] = (axis.x * axis.y) * (1.0f - c) - (axis.z * s);
		mat[1][1] = (axis.y * axis.y) * (1.0f - c) + c;
		mat[1][2] = (axis.z * axis.y) * (1.0f - c) + (axis.x * s);
		 
		mat[2][0] = (axis.x * axis.z) * (1.0f - c) + (axis.y * s);
		mat[2][1] = (axis.y * axis.z) * (1.0f - c) - (axis.x * s);
		mat[2][2] = (axis.z * axis.z) * (1.0f - c) + c;

		return mat;
	}
	glm::quat Transform::EulerAnglesToQuat(float roll, float yaw, float pitch)
	{
		float cos1 = (float)cos(glm::radians(yaw * 0.5f));
		float cos2 = (float)cos(glm::radians(pitch * 0.5f));
		float cos3 = (float)cos(glm::radians(roll * 0.5f));

		float sin1 = (float)sin(glm::radians(yaw * 0.5f));
		float sin2 = (float)sin(glm::radians(pitch * 0.5f));
		float sin3 = (float)sin(glm::radians(roll * 0.5f));

		glm::quat q;

		q.x = (sin1 * sin2 * cos3) + (cos1 * cos2 * sin3);
		q.y = (sin1 * cos2 * cos3) + (cos1 * sin2 * sin3);
		q.z = (cos1 * sin2 * cos3) - (sin1 * cos2 * sin3);
		q.w = (cos1 * cos2 * cos3) - (sin1 * sin2 * sin3);

		return q;
	}
}