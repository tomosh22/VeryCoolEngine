#pragma once

#include "core.h"
#include <iostream>


namespace VeryCoolEngine {

	class VCE_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();
		
	};
	Application* CreateApplication();
	
}
