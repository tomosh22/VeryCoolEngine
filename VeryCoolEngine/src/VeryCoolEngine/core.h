#pragma once

#ifdef VCE_WINDOWS
	#ifdef VCE_BUILD_DLL
		#define VCE_API __declspec(dllexport)
	#else
		#define VCE_API __declspec(dllimport)
	#endif
#else
#error not built for windows
#endif