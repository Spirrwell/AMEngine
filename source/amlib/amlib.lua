project "amlib"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "amlib"

	pic "On"
	
	files { 
			"%{cfg.location}/memoryreader.cpp",
			"%{cfg.location}/memorywriter.cpp",
			"%{cfg.location}/transform.cpp",
			"../shared/amlib/memoryreader.hpp",
			"../shared/amlib/memorywriter.hpp",
		}
		
	includedirs {
				"../shared",
				"../shared/amlib",
				"../thirdparty/glm/include"
		}
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "Full"
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"

	filter { "platforms:Linux64" }
		system "Linux"
		architecture "x64"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "debug_win64"
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "release_win64"

	filter { "platforms:Linux64", "configurations:Debug" }
		targetdir "debug_linux64"

	filter { "platforms:Linux64", "configurations:Release" }
		targetdir "release_linux64"
	
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64" }
		buildoptions{ "/permissive-" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win64/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/shared/win64/release\" /s /i /y" }

	filter { "platforms:Linux64", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux64/debug\"" }

	filter { "platforms:Linux64", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux64/release\"" }