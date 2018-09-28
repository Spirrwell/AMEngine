project "memory_system"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	location "./"
	targetname "memory_system"
	targetprefix ""
	
	defines { "MEMORY_SYSTEM_DLL_EXPORT" }
	
	files {
			"%{cfg.location}/memory_system.hpp",
			"%{cfg.location}/memory_system.cpp"
		}
		
	includedirs {
				"../shared",
				"../shared/memory_system"
		}
	
	links {
			"amlib" --Project
		}
	
	filter { "system:Windows" }
		links {
				"amlib.lib"
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

	filter { "platforms:Linux64", "configurations:Debug" }
		targetdir "debug_linux64"
		architecture "x64"

	filter { "platforms:Linux64", "configurations:Release" }
		architecture "x64"
		targetdir "release_linux64"

	--Library Directories
	filter { "platforms:Win64", "configurations:Debug" }
		libdirs {
				"../lib/shared/win64/debug"
			}
		
	filter { "platforms:Win64", "configurations:Release" }
		libdirs {
				"../lib/shared/win64/release"
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
		defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }

	filter { "action:vs*", "platforms:Win64", "configurations:Debug" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/win64/debug\" /s /i /y", "xcopy \"$(TargetDir)$(TargetName).lib\" \"../lib/shared/win64/debug\" /s /i /y" }

	filter { "action:vs*", "platforms:Win64", "configurations:Release" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/win64/release\" /s /i /y", "xcopy \"$(TargetDir)$(TargetName).lib\" \"../lib/shared/win64/release\" /s /i /y" }

	--Lib
	filter { "platforms:Linux64", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux64/debug\"" }

	filter { "platforms:Linux64", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../lib/shared/linux64/release\"" }

	--Bin
	filter { "platforms:Linux64", "configurations:Debug" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../game/linux64/debug\"" }

	filter { "platforms:Linux64", "configurations:Release" }
		postbuildcommands { "cp \"%{cfg.targetdir}/%{cfg.targetprefix}%{cfg.targetname}%{cfg.targetextension}\" \"../../game/linux64/release\"" }