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
		VCEModel* AddModel(const char* szFileName, Material* pxMaterial, Transform xTrans);

		//TODO: make xTrans a reference
		VCEModel* AddModel(const char* szFileName, Transform xTrans);

		void AddBoxCollisionVolumeToModel(VCEModel* pxModel, glm::vec3 xHalfExtents);
		void AddSphereCollisionVolumeToModel(VCEModel* pxModel, float fRadius);
		void AddCapsuleCollisionVolumeToModel(VCEModel* pxModel, float fRadius, float fHeight);
		
	};


}
