#pragma once
#include "Texture.h"
#include "VeryCoolEngine/PlatformTypes.h"
namespace VeryCoolEngine {
	class Material
	{
	public:
		virtual ~Material() {}

		static Material* Create();
		static Material* Create(const char* szName, GUID xGUID);

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

		bool m_bInitialised = false;
		uint32_t m_uNumTextures = 0;

		std::string m_strName;
		GUID m_xGUID;
	};

	class FoliageMaterial {
	public:
		virtual ~FoliageMaterial() {}

		static FoliageMaterial* Create();
		static FoliageMaterial* Create(const char* szName);

		void SetAlbedo(Texture2D* pxTex);
		void SetBumpMap(Texture2D* pxTex);
		void SetRoughness(Texture2D* pxTex);
		void SetHeightmap(Texture2D* pxTex);
		void SetAlpha(Texture2D* pxTex);
		void SetTranslucency(Texture2D* pxTex);

		virtual void PlatformInit() = 0;

		Texture2D* m_pxAlbedo = nullptr;
		Texture2D* m_pxBumpMap = nullptr;
		Texture2D* m_pxRoughnessTex = nullptr;
		Texture2D* m_pxHeightmapTex = nullptr;
		Texture2D* m_pxAlphaTex = nullptr;
		Texture2D* m_pxTranslucencyTex = nullptr;

		bool m_bInitialised = false;
		uint32_t m_uNumTextures = 0;
	};
}
