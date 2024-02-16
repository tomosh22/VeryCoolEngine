#pragma once
#include "single_include/entt/entt.hpp"
namespace VeryCoolEngine {
	using EntityRegistry = entt::registry;
	using EntityID = entt::entity;
	using GuidType = uint64_t;
	struct GUID {
		GUID() {
			for (uint32_t i = 0; i < sizeof(GuidType) * 8; i++)
				if (rand() > RAND_MAX / 2)
					m_uGuid |= 1ul << i;
		}
		GUID(GuidType uGuid) : m_uGuid(uGuid) {}
		GuidType m_uGuid = 0;
	};
}