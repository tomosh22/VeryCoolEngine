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

		

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Material* pxMaterial, Transform xTrans, bool bPhysics, uint32_t uMeshIndex = 0);

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Transform xTrans, bool bPhysics);
		
	};


}
