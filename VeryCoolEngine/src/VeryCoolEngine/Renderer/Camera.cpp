#include "vcepch.h"
#include "Camera.h"
#include "VeryCoolEngine/Application.h"


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
		double frameSpeed = dt / 50;

		std::pair<double, double> currentMousePos = Input::GetMousePos();
		if (prevMousePos.first == std::numeric_limits<float>::max()) {
			prevMousePos = currentMousePos;
			return;
		}
		if (Application::GetInstance()->_mouseEnabled) {
			

			//std::cout << currentMousePos.first << " " << currentMousePos.second << std::endl;



			double deltaPitch = (currentMousePos.second - prevMousePos.second) / 1000.;
#ifdef VCE_OPENGL
			pitch -= deltaPitch;
#elif defined VCE_VULKAN
			pitch += deltaPitch;
#endif
			double deltaYaw = (currentMousePos.first - prevMousePos.first) / 1000.;
			yaw -= deltaYaw;

			//if(deltaPitch != 0) std::cout << "pitch " << deltaPitch << std::endl;
			//if (deltaYaw != 0)std::cout << "yaw " << deltaYaw << std::endl;
			

			//Bounds check the pitch, to be between straight up and straight down ;)
			pitch = std::min(pitch, glm::pi<double>() / 2);
			pitch = std::max(pitch, -glm::pi<double>() / 2);


			if (yaw < 0) {
				yaw += glm::pi<double>() * 2;
			}
			if (yaw > glm::pi<double>() * 2) {
				yaw -= glm::pi<double>() * 2;
			}
		}
		
		prevMousePos = currentMousePos;

		//std::cout << yaw << std::endl;

		if (Input::IsKeyPressed(VCE_KEY_W)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3( 0,1,0 ));
			glm::vec4 result = rotation * glm::vec4(0, 0, -1,1) * frameSpeed;
			position += glm::vec3(result.x, result.y, result.z);
			//position += glm::mat4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		}
		if (Input::IsKeyPressed(VCE_KEY_S)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3(0, 1, 0));
			glm::vec4 result = rotation * glm::vec4(0, 0, -1, 1) * frameSpeed;
			position -= glm::vec3(result.x, result.y, result.z);
			//position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		}

		if (Input::IsKeyPressed(VCE_KEY_A)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3(0, 1, 0));
			glm::dvec4 result = rotation * glm::vec4(-1, 0, 0, 1) * frameSpeed;
			position += glm::vec3(result.x, result.y, result.z);
			//position += Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		}
		if (Input::IsKeyPressed(VCE_KEY_D)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3(0, 1, 0));
			glm::dvec4 result = rotation * glm::vec4(-1, 0, 0, 1) * frameSpeed;
			position -= glm::vec3(result.x,result.y,result.z);
			//position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		}

		if (Input::IsKeyPressed(VCE_KEY_LEFT_SHIFT)) {
#ifdef VCE_OPENGL
			position.y -= frameSpeed;
#elif defined VCE_VULKAN
			position.y += frameSpeed;
#endif
		}
		if (Input::IsKeyPressed(VCE_KEY_SPACE)) {
#ifdef VCE_OPENGL
			position.y += frameSpeed;
#elif defined VCE_VULKAN
			position.y -= frameSpeed;
#endif
		}
	}

	/*
	Generates a view matrix for the camera's viewpoint. This matrix can be sent
	straight to the shader...it's already an 'inverse camera' matrix.
	*/
	glm::mat4 Camera::BuildViewMatrix() const {
		//Why do a complicated matrix inversion, when we can just generate the matrix
		//using the negative values ;). The matrix multiplication order is important!
		glm::dmat4 pitchMat = glm::rotate(-pitch, glm::dvec3(1, 0, 0));
		glm::dmat4 yawMat = glm::rotate(-yaw, glm::dvec3(0, 1, 0));
		glm::dmat4 transMat = glm::translate(-position);

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

	glm::vec3 Camera::ViewDirection()
	{
		glm::vec3 result;

		result.z = -cos(yaw) * cos(pitch);
		result.x = -sin(yaw) * cos(pitch);
		result.y = sin(pitch);

		return glm::normalize(result);
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
