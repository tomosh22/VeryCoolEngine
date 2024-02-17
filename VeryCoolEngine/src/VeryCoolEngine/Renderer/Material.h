#pragma once
#include "Texture.h"
#include "VeryCoolEngine/PlatformTypes.h"
namespace VeryCoolEngine {
	class Material
	{
	public:
		virtual ~Material() {
		}

		static Material* Create();
		static Material* Create(GUID xAlbedoGUID, GUID xBumpMapGUID, GUID xRoughnessTexGUID, GUID xMetallicTexGUID, GUID xHeightmapTexGUID);

		void SetAlbedo(GUID xGUID);
		void SetBumpMap(GUID xGUID);
		void SetRoughness(GUID xGUID);
		void SetMetallic(GUID xGUID);
		void SetHeightmap(GUID xGUID);

		void SetAlbedo(Texture2D* pxTex);
		void SetBumpMap(Texture2D* pxTex);
		void SetRoughness(Texture2D* pxTexD);
		void SetMetallic(Texture2D* pxTex);
		void SetHeightmap(Texture2D* pxTex);

		virtual void PlatformInit() = 0;


		GUID m_xAlbedoGUID;
		GUID m_xBumpMapGUID;
		GUID m_xRoughnessTexGUID;
		GUID m_xMetallicTexGUID;
		GUID m_xHeightmapTexGUID;

		//does not own
		Texture2D* m_pxAlbedo;
		Texture2D* m_pxBumpMap;
		Texture2D* m_pxRoughnessTex;
		Texture2D* m_pxMetallicTex;
		Texture2D* m_pxHeightmapTex;

		bool m_bInitialised = false;
		uint32_t m_uNumTextures = 0;

		std::string m_strName;
		GUID m_xGUID;
	};

	class FoliageMaterial {
	public:
		virtual ~FoliageMaterial() {
			VCE_DELETE(m_pxAlbedo);
			VCE_DELETE(m_pxBumpMap);
			VCE_DELETE(m_pxRoughnessTex);
			VCE_DELETE(m_pxHeightmapTex);
			VCE_DELETE(m_pxAlphaTex);
			VCE_DELETE(m_pxTranslucencyTex);
		}

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
