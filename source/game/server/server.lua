project "server"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "server"
	targetprefix ""
	
	defines { "SERVER_DLL" }
	
	vpaths {
		[ "Header Files" ] = { "**.hpp", "../shared/**.hpp", "../../shared/**.hpp" },
		[ "Source Files" ] = { "**.cpp", "../shared/**.cpp" }
	}
	
	files {
			"%{cfg.location}/server.hpp",
			"%{cfg.location}/server.cpp",
			"../shared/dummyobject.hpp",
			"../shared/dummyobject.cpp",
			"../shared/gameobject.hpp",
			"../shared/gameobject.cpp",
			"../shared/gameobjectfactory.hpp",
			"../shared/gameobjectfactory.cpp",
			"../shared/networktable.hpp",
			"../shared/networktable.cpp",
			"../shared/networkvar.hpp",
			"../../shared/game/client/iserver.hpp"
		}
		
	includedirs {
				"../shared",
				"../../shared",
				"../../shared/game/server",
				"../../thirdparty/glm/include"
		}
	
	links {
			"amlib", --Project
			"factory", --Project
			"memlib", --Project
			"memory_system" --Project
		}
	
	filter { "system:Windows" }
		links {
				"amlib.lib",
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

	filter { "platforms:Linux64", "configurations:Debug" }
		targetdir "debug_linux64"
		architecture "x64"

	filter { "platforms:Linux64", "configurations:Release" }
		architecture "x64"
		targetdir "release_linux64"

	filter { "platforms:Win64", "configurations:Debug" }
		libdirs {
				"../../lib/shared/win64/debug"
			}
		
	filter { "platforms:Win64", "configurations:Release" }
		libdirs {
				"../../lib/shared/win64/release"
			}

	filter { "platforms:Linux64", "configurations:Debug" }
		libdirs {
				"../lib/shared/linux64/debug"
		}

	filter { "platforms:Linux64", "configurations:Release" }
		libdirs {
				"../lib/shared/linux64/release"
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