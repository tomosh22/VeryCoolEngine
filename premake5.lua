require "cmake"

workspace "VeryCoolEngine"
	architecture "x64"

	configurations{
		"Debug",
		"Release",
		"Dist"
	}

outputDir = "%{cfg.buildcfg}-%{cfg.cystem}-%{cfg.architecture}"

project "VeryCoolEngine"
	location"VeryCoolEngine"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h"
	}

	includedirs{
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"

		defines{
			"VCE_WINDOWS",
			"VCE_BUILD_DLL"
		}

		postbuildcommands{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Game")
		}

	filter "configurations:Debug"
		defines "VCE_DEBUG"
		symbols "On"
	

	filter "configurations:Release"
		defines "VCE_RELEASE"
		optimize "On"
	

	filter "configurations:Dist"
		defines "VCE_DIST"
		optimize "On"
	

project "Game"
	location "Game"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h"
	}

	includedirs{
		"VeryCoolEngine/vendor/spdlog/include",
		"VeryCoolEngine/src"
	}

	links{"VeryCoolEngine"}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0"

		defines{
			"VCE_WINDOWS",
		}

	filter "configurations:Debug"
		defines "VCE_DEBUG"
		symbols "On"
	

	filter "configurations:Release"
		defines "VCE_RELEASE"
		optimize "On"
	

	filter "configurations:Dist"
		defines "VCE_DIST"
		optimize "On"
	