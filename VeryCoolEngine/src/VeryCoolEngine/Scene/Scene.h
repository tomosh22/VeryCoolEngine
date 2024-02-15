#pragma once
#include "VeryCoolEngine/Renderer/Camera.h"
#include "VeryCoolEngine/PlatformTypes.h"

namespace VeryCoolEngine {
	class ColliderComponent;
	class Entity;



	class Scene
	{
	public:
		virtual void Reset() = 0;

		template<typename T>
		T& GetComponentFromEntity(EntityID xID) {
			VCE_ASSERT(EntityHasComponent<T>(xID), "Doesn't have this component");
			return m_xRegistry.get<T>(xID);
		}

		template<typename T>
		bool EntityHasComponent(EntityID xID) const {
			return m_xRegistry.all_of<T>(xID);
		}

		template<typename T>
		std::vector<T*> GetAllOfComponentType() {
			std::vector<T*> xRet;
			auto view = m_xRegistry.view<T>();
			for (auto [xEntity, xComponent] : view.each())
				xRet.push_back(&xComponent);
			return xRet;
		}

		void Serialize(const std::string& strFilename);
		
		std::vector<ColliderComponent*> GetAllColliderComponents();

		Entity GetEntityByGuid(GUID xGuid);
		Entity GetEntityByGuid(GuidType uGuid);

		Camera m_xEditorCamera;
		Camera m_xGameCamera;
	private:
		friend class Entity;
		EntityRegistry m_xRegistry;
		std::unordered_map<GuidType, Entity> m_xEntityMap;
	};

}

