
workspace "VeryCoolEngine"
	architecture "x64"

	configurations{
		"Debug",
		"Release",
		"Dist"
	}

outputDir = "%{cfg.buildcfg}-%{cfg.cystem}-%{cfg.architecture}"

include "VeryCoolEngine/vendor/GLFW"
include "VeryCoolEngine/vendor/imgui"

project "VeryCoolEngine"
	location"VeryCoolEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	pchheader "vcepch.h"
	pchsource "VeryCoolEngine/src/vcepch.cpp"

	assetPath = "/Assets/"
	defines{
		"ASSETROOTLOCATION=" .. '\"' .. _WORKING_DIR .. assetPath .. '\"',
		"VCE_VULKAN"
	}

	files{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}
	files{ "%{prj.name}/vendor/imgui/backends/imgui_impl_vulkan.cpp", "%{prj.name}/vendor/imgui/backends/imgui_impl_vulkan.h"}
		flags{"NoPCH"}

	includedirs{
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/GLFW/include",
		"%{prj.name}/vendor/Glad/include",
		"%{prj.name}/vendor/imgui",
		"%{prj.name}/vendor/glm",
		"%{prj.name}/vendor/stb",
		"%{prj.name}/src",
		"$(VULKAN_SDK)/include",
	}
	libdirs {
		"$(VULKAN_SDK)/lib"
	}
	links{
		"GLFW",
		"opengl32.lib",
		"ImGui",
		"vulkan-1.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines{
			"VCE_WINDOWS",
			"VCE_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
		}

		

	filter "configurations:Debug"
		defines "VCE_DEBUG"
		symbols "on"
	

	filter "configurations:Release"
		defines "VCE_RELEASE"
		optimize "on"
	

	filter "configurations:Dist"
		defines "VCE_DIST"
		optimize "on"
	

project "Game"
	location "Game"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")
	
	

	files{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h"
	}

	includedirs{
		"VeryCoolEngine/vendor/spdlog/include",
		"VeryCoolEngine/src",
		"VeryCoolEngine/vendor/imgui",
		"VeryCoolEngine/vendor/glm",
		"VeryCoolEngine/vendor/Glad/include",
		"%{prj.name}/vendor/stb",
		"Game/vendor/PerlinNoise",
		"$(VULKAN_SDK)/include",

	}

	links{"VeryCoolEngine"}

	filter "system:windows"
		systemversion "latest"

		defines{
			"VCE_WINDOWS",
		}

	filter "configurations:Debug"
		defines "VCE_DEBUG"
		symbols "on"
	

	filter "configurations:Release"
		defines "VCE_RELEASE"
		optimize "on"
	

	filter "configurations:Dist"
		defines "VCE_DIST"
		optimize "on"
	