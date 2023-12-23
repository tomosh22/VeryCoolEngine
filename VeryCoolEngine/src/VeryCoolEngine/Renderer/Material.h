#pragma once
#include "Texture.h"
namespace VeryCoolEngine {
	class Material
	{
	public:
		virtual ~Material() {}

		static Material* Create();

		void SetAlbedo(Texture2D* pxTex);
		void SetBumpMap(Texture2D* pxTex);
		void SetRoughness(Texture2D* pxTex);
		void SetMetallic(Texture2D* pxTex);
		void SetHeightmap(Texture2D* pxTex);

		virtual void PlatformInit() = 0;

		Texture2D* m_pxAlbedo = nullptr;
		Texture2D* m_pxBumpMap = nullptr;
		Texture2D* m_pxRoughnessTex = nullptr;
		Texture2D* m_pxMetallicTex = nullptr;
		Texture2D* m_pxHeightmapTex = nullptr;

	protected:
		bool m_bInitialised = false;
		uint32_t m_uNumTextures = 0;
	};
}
