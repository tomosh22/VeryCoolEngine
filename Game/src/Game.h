#pragma once

#include "VeryCoolEngine/Application.h"
#include <glm/glm.hpp>
#include "VeryCoolEngine/BlockWorld/Block.h"
#include "VeryCoolEngine/BlockWorld/Chunk.h"
#include <map>

namespace VeryCoolEngine {

	

	class Game : public Application {
	public:
		Game();
		~Game();

		void GenerateChunks();
		void UploadChunks();

		


		VCEModel* AddModel(const char* szFileName, Material* pxMaterial, const Transform& xTrans, uint32_t uMeshIndex = 0);
		
		VCEModel* AddModel(const char* szFileName, const Transform& xTrans);
		
	};


}
