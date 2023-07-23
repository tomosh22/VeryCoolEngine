#include "vcepch.h"
#include "Camera.h"



#include <algorithm>

/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/

/*
Polls the camera for keyboard / mouse movement.
Should be done once per frame! Pass it the msec since
last frame (default value is for simplicities sake...)
*/

namespace VeryCoolEngine {
	void Camera::UpdateCamera(float dt) {
		//Update the mouse by how much
		std::pair<float, float> currentMousePos = Input::GetMousePos();
		if (prevMousePos.first == std::numeric_limits<float>::max()) {
			prevMousePos = currentMousePos;
			return;
		}
		

		float frameSpeed = dt/50;

		pitch -= (currentMousePos.second/100 - prevMousePos.second/100) * frameSpeed;
		yaw -= (currentMousePos.first/100 - prevMousePos.first/100) * frameSpeed;
		prevMousePos = currentMousePos;

		//Bounds check the pitch, to be between straight up and straight down ;)
		pitch = std::min(pitch, glm::pi<float>() / 2);
		pitch = std::max(pitch, -glm::pi<float>() / 2);


		if (yaw < 0) {
			yaw += glm::pi<float>() * 2;
		}
		if (yaw > glm::pi<float>() * 2) {
			yaw -= glm::pi<float>() * 2;
		}

		//std::cout << yaw << std::endl;

		if (Input::IsKeyPressed(VCE_KEY_W)) {
			glm::mat4 rotation = glm::rotate(yaw, glm::vec3( 0,1,0 ));
			glm::vec4 result = rotation * glm::vec4(0, 0, -1,1) * frameSpeed;
			position += glm::vec3(result.x, result.y, result.z);
			//position += glm::mat4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		}
		if (Input::IsKeyPressed(VCE_KEY_S)) {
			glm::mat4 rotation = glm::rotate(yaw, glm::vec3(0, 1, 0));
			glm::vec4 result = rotation * glm::vec4(0, 0, -1, 1) * frameSpeed;
			position -= glm::vec3(result.x, result.y, result.z);
			//position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		}

		if (Input::IsKeyPressed(VCE_KEY_A)) {
			glm::mat4 rotation = glm::rotate(yaw, glm::vec3(0, 1, 0));
			glm::vec4 result = rotation * glm::vec4(-1, 0, 0, 1) * frameSpeed;
			position += glm::vec3(result.x, result.y, result.z);
			//position += Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		}
		if (Input::IsKeyPressed(VCE_KEY_D)) {
			glm::mat4 rotation = glm::rotate(yaw, glm::vec3(0, 1, 0));
			glm::vec4 result = rotation * glm::vec4(-1, 0, 0, 1) * frameSpeed;
			position -= glm::vec3(result.x,result.y,result.z);
			//position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		}

		if (Input::IsKeyPressed(VCE_KEY_LEFT_SHIFT)) {
			position.y -= frameSpeed;
		}
		if (Input::IsKeyPressed(VCE_KEY_SPACE)) {
			position.y += frameSpeed;
		}
	}

	/*
	Generates a view matrix for the camera's viewpoint. This matrix can be sent
	straight to the shader...it's already an 'inverse camera' matrix.
	*/
	glm::mat4 Camera::BuildViewMatrix() const {
		//Why do a complicated matrix inversion, when we can just generate the matrix
		//using the negative values ;). The matrix multiplication order is important!
		glm::mat4 pitchMat = glm::rotate(-pitch, glm::vec3(1, 0, 0));
		glm::mat4 yawMat = glm::rotate(-yaw, glm::vec3(0, 1, 0));
		glm::mat4 transMat = glm::translate(-position);

		return pitchMat * yawMat * transMat;

		//return	Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
			//Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
			//Matrix4::Translation(-position);
	};

	glm::mat4 Camera::BuildProjectionMatrix() const {
		if (camType == CameraType::Orthographic) {
			return glm::ortho(left, right, bottom, top, nearPlane,farPlane);
			//return Matrix4::Orthographic(left, right, bottom, top, nearPlane, farPlane);
		}
		else if (camType == CameraType::Perspective) {
			return glm::perspective(fov, _aspectRatio, nearPlane, farPlane);
			//return Matrix4::Perspective(nearPlane, farPlane, currentAspect, fov);
		}
	}

	Camera Camera::BuildPerspectiveCamera(const glm::vec3& pos, float pitch, float yaw, float fov, float nearPlane, float farPlane, float aspectRatio) {
		Camera c;
		c.camType = CameraType::Perspective;
		c.position = pos;
		c.pitch = pitch;
		c.yaw = yaw;
		c.nearPlane = nearPlane;
		c.farPlane = farPlane;

		c.fov = fov;

		c._aspectRatio = aspectRatio;

		return c;
	}
	Camera Camera::BuildOrthoCamera(const glm::vec3& pos, float pitch, float yaw, float left, float right, float top, float bottom, float nearPlane, float farPlane) {
		Camera c;
		c.camType = CameraType::Orthographic;
		c.position = pos;
		c.pitch = pitch;
		c.yaw = yaw;
		c.nearPlane = nearPlane;
		c.farPlane = farPlane;

		c.left = left;
		c.right = right;
		c.top = top;
		c.bottom = bottom;

		return c;
	}
}
