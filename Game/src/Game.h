#pragma once

#include "VeryCoolEngine/Application.h"
#include <glm/glm.hpp>
#include "VeryCoolEngine/BlockWorld/Block.h"
#include "VeryCoolEngine/BlockWorld/Chunk.h"
#include <map>

#include "VeryCoolEngine/Physics/Physics.h"

namespace VeryCoolEngine {

	class Game : public Application {
	public:

		Game();
		~Game();

		void GenerateChunks();
		void UploadChunks();

		

		

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Material* pxMaterial, Transform xTrans);

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Transform xTrans);

		

		VCEModel* m_pxPlayerModel;
		VCEModel* m_pxGroundPlane;

		bool m_bPlayerIsOnFloor = false;
	};


}
