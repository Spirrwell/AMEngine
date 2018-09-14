project "engine"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "engine"
	
	vpaths {
		[ "Header Files" ] = { "**.hpp", "../shared/**.hpp", "../shared/engine/**.hpp" },
		[ "Source Files" ] = { "**.cpp" }
	}
	
	files {
			"%{cfg.location}/engine.hpp",
			"%{cfg.location}/engine.cpp",
			"%{cfg.location}/engineclient.hpp",
			"%{cfg.location}/engineclient.cpp",
			"%{cfg.location}/enginelauncher.hpp",
			"%{cfg.location}/enginelauncher.cpp",
			"%{cfg.location}/engineserver.hpp",
			"%{cfg.location}/engineserver.cpp",
			"%{cfg.location}/netclient.hpp",
			"%{cfg.location}/netclient.cpp",
			"%{cfg.location}/nethost.hpp",
			"%{cfg.location}/nethost.cpp",
			"%{cfg.location}/netmessage.hpp",
			"%{cfg.location}/netmessage.cpp",
			"%{cfg.location}/netobject.hpp",
			"%{cfg.location}/netobject.cpp",
			"../shared/engine/iengineclient.hpp",
			"../shared/engine/ienginelauncher.hpp",
			"../shared/engine/iengineserver.hpp",
			"../shared/engine/igameobject.hpp",
			"../shared/engine/igameobjectfactory.hpp",
			"../shared/engine/inetobject.hpp",
			"../shared/engine/inetworktable.hpp",
			"../shared/engine/inetworkvar.hpp"
		}
		
	includedirs {
				"../shared",
				"../shared/engine",
				"../thirdparty/enet-1.3.13/include",
				"../thirdparty/glm/include",
				"../thirdparty/SDL2-2.0.8/include"
		}
	
	links {
			"amlib", --Project
			"factory", --Project
			"memlib", --Project
			"memory_system", --Project
			"sdl_core" --Project
		}
	
	filter { "system:Windows" }
		links {
				"amlib.lib",
				"factory.lib",
				"memlib.lib",
				"memory_system.lib",
				"SDL2main.lib",
				"SDL2.lib",
				"sdl_core.lib",
				"ws2_32.lib",
				"winmm.lib"
			}
	
	filter { "platforms:Win64" }
		links {
				"enet64.lib"
		}
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "debug_win64"
		architecture "x64"
		debugcommand "../../game/win64/debug/launcher.exe"
		debugdir "../../game/win64/debug"

	filter { "platforms:Win64", "configurations:Release" }
		targetdir "release_win64"
		architecture "x64"
		debugcommand "../../game/win64/release/launcher.exe"
		debugdir "../../game/win64/release"

	--Library Directories
	filter { "platforms:Win64", "configurations:Debug" }
		libdirs {
				"../lib/shared/win64/debug",
				"../lib/thirdparty/win64/debug",
				"../lib/thirdparty/win64"
			}
		
	filter { "platforms:Win64", "configurations:Release" }
		libdirs {
				"../lib/shared/win64/release",
				"../lib/thirdparty/win64/release",
				"../lib/thirdparty/win64"
			}

	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/win64/debug/bin\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/win64/release/bin\" /s /i /y" }