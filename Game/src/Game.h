#pragma once

#include "VeryCoolEngine/Application.h"
#include <glm/glm.hpp>
#include <map>

#include "VeryCoolEngine/Physics/Physics.h"

namespace VeryCoolEngine {

	class Game : public Application {
	public:

		Game();
		~Game();

		void GenerateChunks();
		void UploadChunks();

		

		

		

		
	};


}
