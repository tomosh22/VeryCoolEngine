
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
include "VeryCoolEngine/vendor/assimp"
include "VeryCoolEngine/vendor/reactphysics3d"

project "VeryCoolEngine"
	location"VeryCoolEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

	pchheader "vcepch.h"
	pchsource "%{prj.name}/src/vcepch.cpp"

	assetPath = "/Assets/"
	defines{
		"ASSETROOTLOCATION=" .. '\"' .. _WORKING_DIR .. assetPath .. '\"',
		"VCE_VULKAN",
		"NOMINMAX"
	}

	files{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}
		

	includedirs{
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/GLFW/include",
		"%{prj.name}/vendor/Glad/include",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/imgui",
		"%{prj.name}/vendor/glm",
		"%{prj.name}/vendor/stb",
		"%{prj.name}/vendor/tinyobj",
		"%{prj.name}/vendor/PerlinNoise",
		"VeryCoolEngine/vendor/entt",
		"%{prj.name}/src",
		"%{prj.name}/vendor/reactphysics3d/include",
		"$(VULKAN_SDK)/include",
		
		"Tools"
	}
	libdirs {
		"$(VULKAN_SDK)/lib"
	}
	links{
		"GLFW",
		"ImGui",
		"vulkan-1.lib",
		"assimp",
		"Tools"
	}

	filter "system:windows"
		systemversion "latest"

		defines{
			"VCE_WINDOWS",
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
		"VeryCoolEngine/vendor/entt",
		"VeryCoolEngine/vendor/Glad/include",
		"VeryCoolEngine/vendor/reactphysics3d/include",
		"%{prj.name}/vendor/stb",
		"$(VULKAN_SDK)/include",

	}

	links{"VeryCoolEngine", "reactphysics3d"}

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
		
project "Tools"
	location "Tools"
	kind "StaticLib"
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
		"$(OPENCV_DIR)/../../include",
		"VeryCoolEngine/src",
		"VeryCoolEngine/vendor/spdlog/include",
		"$(VULKAN_SDK)/include",
		"VeryCoolEngine/vendor/glm",
		"VeryCoolEngine/vendor/entt",
	}
	
	
	
	defines{
			"VCE_WINDOWS"
		}
		
		filter "configurations:Debug"
		defines "VCE_DEBUG"
		symbols "on"
		links{
			"$(OPENCV_DIR)/lib/opencv_world490d.lib"
		}
	

	filter "configurations:Release"
		defines "VCE_RELEASE"
		optimize "on"
		links{
			"$(OPENCV_DIR)/lib/opencv_world490.lib"
		}
	

	filter "configurations:Dist"
		defines "VCE_DIST"
		optimize "on"
		links{
			"$(OPENCV_DIR)/lib/opencv_world490.lib"
		}