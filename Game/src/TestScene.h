#pragma once
#include "VeryCoolEngine/Scene/Scene.h"
#include "VeryCoolEngine/Scene/Entity.h"

namespace VeryCoolEngine {
	class TestScene : public Scene
	{
	public:
		TestScene();
		void Reset() override;

		Entity m_xPlayerEntity;
		Entity m_xPlaneEntity;
		Entity m_xSphereEntity;
	};
}

