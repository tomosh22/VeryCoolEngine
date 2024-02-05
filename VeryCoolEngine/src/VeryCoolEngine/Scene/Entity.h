#pragma once
#include "single_include/entt/entt.hpp"

namespace VeryCoolEngine {
	class Scene;
	using EntityRegistry = entt::registry;

	class Entity
	{
	public:
		Entity();

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T>
		bool HasComponent();

		template<typename T>
		T& GetComponent();

		template<typename T>
		void RemoveComponent();

	private:
		entt::entity m_xEntity;
		Scene* m_pxParentScene;
	};

}

