workspace "AMEngine"
	configurations { "Debug", "Release" }
	startproject "client"
	filter { "system:Windows" }
		platforms { "Win64" }
	filter { "action:vs" }
		toolset "v141"

include "amlib/amlib.lua"
include "shadersystem/amshaderlib/amshaderlib.lua"
include "game/client/client.lua"
include "engine/engine.lua"
include "factory/factory.lua"
include "input/input.lua"
include "launcher/launcher.lua"
include "materialsystem/materialsystem.lua"
include "renderer_opengl/renderer_opengl.lua"
include "game/server/server.lua"
include "shadersystem/shadergl/shadergl.lua"
include "shadersystem/standard_shaders/standard_shaders.lua"