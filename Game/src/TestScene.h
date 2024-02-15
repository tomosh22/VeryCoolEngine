#pragma once
#include "VeryCoolEngine/Scene/Scene.h"


namespace VeryCoolEngine {
	class TestScene : public Scene
	{
	public:
		TestScene();
		void Reset() override;

		GUID m_xPlayerGuid;
		GUID m_xPlaneGuid;
		GUID m_xSphereGuid;
	};
}

