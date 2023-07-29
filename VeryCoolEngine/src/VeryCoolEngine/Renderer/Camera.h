/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Credit Rich Davison
*/
#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include "VeryCoolEngine/Input.h"
#include "VeryCoolEngine/KeyCodes.h"

namespace VeryCoolEngine {
	enum class CameraType {
		Orthographic,
		Perspective
	};

	class Camera {
	public:
		Camera(void) {
			left = 0;
			right = 0;
			top = 0;
			bottom = 0;

			pitch = 0.0f;
			yaw = 0.0f;

			fov = 45.0f;
			nearPlane = 1.0f;
			farPlane = 100.0f;

			camType = CameraType::Perspective;
		};

		Camera(float pitch, float yaw, const glm::vec3& position) : Camera() {
			this->pitch = pitch;
			this->yaw = yaw;
			this->position = position;

			this->fov = 45.0f;
			this->nearPlane = 1.0f;
			this->farPlane = 100.0f;

			this->camType = CameraType::Perspective;
		}

		~Camera(void) = default;

		void UpdateCamera(float dt);

		float GetFieldOfVision() const {
			return fov;
		}

		float GetNearPlane() const {
			return nearPlane;
		}

		float GetFarPlane() const {
			return farPlane;
		}

		Camera& SetNearPlane(float val) {
			nearPlane = val;
			return *this;
		}

		Camera& SetFarPlane(float val) {
			farPlane = val;
			return *this;
		}

		//Builds a view matrix for the current camera variables, suitable for sending straight
		//to a vertex shader (i.e it's already an 'inverse camera matrix').
		glm::mat4 BuildViewMatrix() const;

		glm::mat4 BuildProjectionMatrix() const;

		//Gets position in world space
		glm::vec3 GetPosition() const { return position; }
		//Sets position in world space
		Camera& SetPosition(const glm::vec3& val) { position = val;  return *this; }

		//Gets yaw, in degrees
		float	GetYaw()   const { return yaw; }
		//Sets yaw, in degrees
		Camera& SetYaw(float y) { yaw = y;  return *this; }

		//Gets pitch, in degrees
		float	GetPitch() const { return pitch; }
		//Sets pitch, in degrees
		Camera& SetPitch(float p) { pitch = p; return *this; }

		static Camera BuildPerspectiveCamera(const glm::vec3& pos, float pitch, float yaw, float fov, float near, float far, float aspectRatio);
		static Camera BuildOrthoCamera(const glm::vec3& pos, float pitch, float yaw, float left, float right, float top, float bottom, float near, float far);
	protected:
		CameraType camType;

		float	nearPlane;
		float	farPlane;
		float	left;
		float	right;
		float	top;
		float	bottom;

		float	fov;
		double	yaw;
		double	pitch;

		float _aspectRatio;

		glm::vec3 position;

		std::pair<double, double> prevMousePos = {std::numeric_limits<float>::max(),0};
	};
}