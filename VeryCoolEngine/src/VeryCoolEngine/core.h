#pragma once


//#define VCE_USE_EDITOR
#define VCE_GAME_WIDTH 1280
#define VCE_GAME_HEIGHT 720
#define VCE_EDITOR_ADDITIONAL_WIDTH 400
#define VCE_EDITOR_ADDITIONAL_HEIGHT 75

#ifdef VCE_WINDOWS
#if VCE_DYNAMIC_LINK
	#ifdef VCE_BUILD_DLL
		#define VCE_API __declspec(dllexport)
		#define IMGUI_API __declspec( dllexport )
	#else
		#define VCE_API __declspec(dllimport)
		#define IMGUI_API __declspec( dllimport )
	#endif
#else
#define VCE_API
#endif
#else
#error not built for windows
#endif

#ifdef VCE_DEBUG
#define VCE_ASSERT(x,...){if(!(x)){VCE_ERROR("Assertion failed {0}",__VA_ARGS__);__debugbreak();}}
#define VCE_CORE_ASSERT(x,...){if(!(x)){VCE_CORE_ERROR("Assertion failed {0}",__VA_ARGS__);__debugbreak();}}
#else
#define VCE_ASSERT(x,...)
#define VCE_CORE_ASSERT(x,...)
#endif