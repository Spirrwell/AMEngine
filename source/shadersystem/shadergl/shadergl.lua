project "shadergl"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "shadergl"
	targetprefix ""
	
	vpaths {
		[ "Header Files" ] = { "**.hpp", "../../shared/**.hpp" },
		[ "Source Files" ] = { "**.cpp" }
	}
	
	files {
			"%{cfg.location}/shadergl.hpp",
			"%{cfg.location}/shadergl.cpp",
			"%{cfg.location}/shadermanagergl.hpp",
			"%{cfg.location}/shadermanagergl.cpp",
		}
		
	includedirs {
				"../../shared",
				"../../shared/shadersystem",
				"../../thirdparty/glm/include",
				--"../../thirdparty/assimp-4.0.1/include",
				"../../thirdparty/glew-2.1.0/include"
		}
	
	links {
			"amlib", --Project
			"factory", --Project
			"memlib", --Project
			"memory_system", --Project
			"sdl_core" --Project
		}
	
	filter { "system:Windows" }
		includedirs {
					"../../thirdparty/SDL2-2.0.8/include"
		}
		links {
				"amlib.lib",
				"factory.lib",
				"memlib.lib",
				"memory_system.lib",
				"SDL2main.lib",
				"SDL2.lib",
				"sdl_core.lib",
				"opengl32.lib"
			}

	filter { "system:Linux" }
		includedirs {
					"/usr/include/SDL2" --Yuck
		}
		links {
				"SDL2main",
				"SDL2",
				"GL",
				"GLEW"
		}

	filter { "system:Windows", "configurations:Debug" }
		links {
				"glew32d.lib"
		}

	filter { "system:Windows", "configurations:Release" }
		links {
				"glew32.lib"
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

	filter { "platforms:Linux64", "configurations:Debug" }
		targetdir "debug_linux64"
		architecture "x64"

	filter { "platforms:Linux64", "configurations:Release" }
		architecture "x64"
		targetdir "release_linux64"

	filter { "platforms:Win64", "configurations:Debug" }
		libdirs {
				"../../lib/shared/win64/debug",
				"../../lib/thirdparty/win64/debug"
			}
		
	filter { "platforms:Win64", "configurations:Release" }
		libdirs {
				"../../lib/shared/win64/release",
				"../../lib/thirdparty/win64/release"
			}

	filter { "platforms:Linux64", "configurations:Debug" }
		libdirs {
				"../../lib/shared/linux64/debug",
				"../../lib/thirdparty/linux64/debug"
		}

	filter { "platforms:Linux64", "configurations:Release" }
		libdirs {
				"../../lib/shared/linux64/release",
				"../../lib/thirdparty/linux64/release"
		}

	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../../game/win64/debug/bin\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../../game/win64/release/bin\" /s /i /y" }

	--Bin
	filter { "platforms:Linux64", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../../game/linux64/debug/bin\"" }

	filter { "platforms:Linux64", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../../game/linux64/release/bin\"" }