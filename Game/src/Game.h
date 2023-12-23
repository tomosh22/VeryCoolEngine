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

		


		Mesh* AddTestMesh(const char* szFileName, char* szMaterialName, const Transform& xTrans, uint32_t uMeshIndex = 0);
		

		
	};


}
