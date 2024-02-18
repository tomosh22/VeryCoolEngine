#include "vcepch.h"
#include "Camera.h"
#include "VeryCoolEngine/Application.h"


#include <algorithm>

//#TO_TODO make this platform agnostic
#ifdef VCE_VULKAN
#include "Platform/Vulkan/VulkanRenderer.h"
#endif

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
		double frameSpeed = dt;

		
		UpdateRotation();

		//std::cout << yaw << std::endl;

		if (Input::IsKeyDown(VCE_KEY_W)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3( 0,1,0 ));
			glm::vec4 result = rotation * glm::vec4(0, 0, -1,1) * frameSpeed;
			position += glm::vec3(result.x, result.y, result.z);
			//position += glm::mat4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		}
		if (Input::IsKeyDown(VCE_KEY_S)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3(0, 1, 0));
			glm::vec4 result = rotation * glm::vec4(0, 0, -1, 1) * frameSpeed;
			position -= glm::vec3(result.x, result.y, result.z);
			//position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		}

		if (Input::IsKeyDown(VCE_KEY_A)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3(0, 1, 0));
			glm::dvec4 result = rotation * glm::vec4(-1, 0, 0, 1) * frameSpeed;
			position += glm::vec3(result.x, result.y, result.z);
			//position += Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		}
		if (Input::IsKeyDown(VCE_KEY_D)) {
			glm::dmat4 rotation = glm::rotate(yaw, glm::dvec3(0, 1, 0));
			glm::dvec4 result = rotation * glm::vec4(-1, 0, 0, 1) * frameSpeed;
			position -= glm::vec3(result.x,result.y,result.z);
			//position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		}

		if (Input::IsKeyDown(VCE_KEY_LEFT_SHIFT)) {
			position.y -= frameSpeed;
		}
		if (Input::IsKeyDown(VCE_KEY_SPACE)) {
			position.y += frameSpeed;
		}
	}

	void Camera::UpdateRotation() {

		std::pair<double, double> currentMousePos = Input::GetMousePos();
		if (prevMousePos.first == std::numeric_limits<float>::max()) {
			prevMousePos = currentMousePos;
			return;
		}

		if (Application::GetInstance()->_mouseEnabled) {

			double deltaPitch = (currentMousePos.second - prevMousePos.second) / 1000.;
			pitch -= deltaPitch;
			double deltaYaw = (currentMousePos.first - prevMousePos.first) / 1000.;
			yaw -= deltaYaw;

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
	}

	bool Camera::IsCursorInRendererViewport()
	{
#ifdef VCE_USE_EDITOR
		//#TO_TODO: what happens on window resize? and what about padding from ImGui elements
		return prevMousePos.first < VCE_GAME_WIDTH && prevMousePos.second < VCE_GAME_HEIGHT;
#else
		return true;
#endif
	}

	glm::vec3 Camera::ScreenSpaceToWorldSpace(glm::vec3 xScreenSpace)
	{
		Application* pxApp = Application::GetInstance();
		//#TO_TODO make this platform agnostic
#ifdef VCE_VULKAN
		VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
		xScreenSpace.y = pxRenderer->m_height - xScreenSpace.y;
#endif

		//#TO_TODO: adjust for viewport not taking up whole window in editor mode
		glm::vec2 xScreenSize = { pxRenderer->m_width, pxRenderer->m_height };

		glm::mat4 xInvViewProj = glm::inverse(BuildViewMatrix()) * glm::inverse(BuildProjectionMatrix());

		glm::vec4 xClipSpace = {
			(xScreenSpace.x / xScreenSize.x) * 2.0f - 1.0f,
			(xScreenSpace.y / xScreenSize.y) * 2.0f - 1.0f,
			(xScreenSpace.z),
			1.0f
		};

		glm::vec4 xWorldSpacePreDivide = xInvViewProj * xClipSpace;

		glm::vec3 xWorldSpace = {
			xWorldSpacePreDivide.x / xWorldSpacePreDivide.w,
			xWorldSpacePreDivide.y / xWorldSpacePreDivide.w,
			xWorldSpacePreDivide.z / xWorldSpacePreDivide.w
		};

		return xWorldSpace;
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
