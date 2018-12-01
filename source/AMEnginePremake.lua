workspace "AMEngine"
	configurations { "Debug", "Release" }
	startproject "client"
	filter { "system:Windows" }
		platforms { "Win64" }
    filter { "system:Linux" }
        platforms { "Linux64" }
	filter { "action:vs" }
		toolset "v141"
    filter { "action:codelite" }
		toolset "gcc"
	filter { "toolset:gcc" }
		buildoptions{ "-fno-gnu-unique" } --Hack: Makes dlclose unload .so files

include "amdlbuild/amdlbuild.lua"
include "amlib/amlib.lua"
include "shadersystem/amshaderlib/amshaderlib.lua"
include "game/client/client.lua"
include "engine/engine.lua"
include "factory/factory.lua"
include "input/input.lua"
include "launcher/launcher.lua"
include "materialsystem/materialsystem.lua"
include "memlib/memlib.lua"
include "memory_system/memory_system.lua"
include "renderer_opengl/renderer_opengl.lua"
include "renderer_empty/renderer_empty.lua"
include "renderer_vulkan/renderer_vulkan.lua"
include "sdl_core/sdl_core.lua"
include "game/server/server.lua"
include "shadersystem/shadergl/shadergl.lua"
include "shadersystem/standard_shaders/standard_shaders.lua"