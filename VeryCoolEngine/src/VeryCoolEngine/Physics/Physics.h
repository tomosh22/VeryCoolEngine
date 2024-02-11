#pragma once
#include "reactphysics3d/reactphysics3d.h"
#include "glm/glm.hpp"

namespace VeryCoolEngine {
	class VCEModel;
	class Camera;
	class Application;
	namespace Physics
	{

		enum class CollisionVolumeType : uint32_t {
			AABB,
			OBB,
			Sphere,
			Capsule
		};

		enum class CollisionEventType : uint32_t {
			Start,
			Exit,
			Stay
		};

		extern reactphysics3d::PhysicsCommon s_xPhysicsCommon;
		extern reactphysics3d::PhysicsWorld* s_pxPhysicsWorld;


		void InitPhysics();
		void UpdatePhysics();
		void ResetPhysics();

		reactphysics3d::Ray BuildRayFromMouse(Camera* pxCam);

		extern double s_fTimestepAccumulator;
		//#TO_TODO: make this a define
		constexpr double s_fDesiredFramerate = 1. / 60.;


		class PhysicsEventListener : public reactphysics3d::EventListener {
		public:
			PhysicsEventListener() = default;
			PhysicsEventListener(VeryCoolEngine::Application* pxApp);
			void onContact(const CollisionCallback::CallbackData& xCallbackData) override;
			VeryCoolEngine::Application* m_pxApp;
		};

		extern PhysicsEventListener s_xEventListener;

	}
}