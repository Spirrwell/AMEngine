project "standard_shaders"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "standard_shaders"
	
	vpaths {
		[ "Header Files" ] = { "**.hpp", "../../shared/**.hpp" },
		[ "Source Files" ] = { "**.cpp" }
	}
	
	files {
			"%{cfg.location}/baseshader.cpp",
			"%{cfg.location}/basicshader.hpp",
			"%{cfg.location}/basicshader.cpp",
			"%{cfg.location}/skyshader.hpp",
			"%{cfg.location}/skyshader.cpp"
		}
		
	includedirs {
				"../../shared",
				"../../shared/shadersystem",
				"../../thirdparty/glm/include"
		}
	
	links {
			"amshaderlib", --Project
			"factory", --Project
			"memlib", --Project
			"memory_system" --Project
		}
	
	filter { "system:Windows" }
		links {
				"amshaderlib.lib",
				"factory.lib",
				"memlib.lib",
				"memory_system.lib"
			}
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "debug_win64"
		architecture "x64"
		debugcommand "../../../game/win64/debug/launcher.exe"
		debugdir "../../../game/win64/debug"

	filter { "platforms:Win64", "configurations:Release" }
		targetdir "release_win64"
		architecture "x64"
		debugcommand "../../../game/win64/release/launcher.exe"
		debugdir "../../../game/win64/release"

	filter { "platforms:Win64", "configurations:Debug" }
		libdirs {
				"../../lib/shared/win64/debug"
			}
		
	filter { "platforms:Win64", "configurations:Release" }
		libdirs {
				"../../lib/shared/win64/release"
			}

	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../../game/win64/debug/bin\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../../game/win64/release/bin\" /s /i /y" }