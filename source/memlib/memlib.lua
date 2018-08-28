project "memlib"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "memlib"
	
	files { 
			"%{cfg.location}/memoryoverride.cpp",
			"../shared/memlib/memoryoverride.hpp"
		}
		
	includedirs {
				"../shared",
				"../shared/memlib"
		}
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "debug_win64"
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "release_win64"
	
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64" }
		buildoptions{ "/permissive-" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win64/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win64/release\" /s /i /y" }