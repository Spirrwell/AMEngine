project "launcher"
	kind "ConsoleApp" --Change to WindowedApp at some point
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "launcher"
	
	vpaths {
		[ "Header Files" ] = { "**.hpp", "../shared/**.hpp" },
		[ "Source Files" ] = { "**.cpp" }
	}
	
	files {
			"%{cfg.location}/main.cpp"
		}
		
	includedirs {
				"../shared"
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
				"../lib/shared/win64/debug"
			}
		
	filter { "platforms:Win64", "configurations:Release" }
		libdirs {
				"../lib/shared/win64/release"
			}

	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/win64/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/win64/release\" /s /i /y" }