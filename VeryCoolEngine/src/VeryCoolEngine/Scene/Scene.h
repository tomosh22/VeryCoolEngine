#pragma once
#include "Entity.h"

namespace VeryCoolEngine {

	class Scene
	{
	private:
		friend class Entity;
		EntityRegistry m_xRegistry;
	};

}

