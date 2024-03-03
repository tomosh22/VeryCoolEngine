#pragma once
#include "VeryCoolEngine/PlatformTypes.h"
namespace VeryCoolEngine {
	class Texture2D;
	class HeightmapTexture
	{
	public:
		virtual ~HeightmapTexture() {}


		static HeightmapTexture* Create(GUID xGUID);

		virtual void PlatformInit() = 0;

		void SetTexture(GUID xGUID);

		GUID m_xTextureGUID;
		Texture2D* m_pxTexture;

		bool m_bInitialised = false;
	};
}