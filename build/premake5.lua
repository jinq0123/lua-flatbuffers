-- premake5.lua
--[[
Usage: 
	windows: premake5.exe --os=windows vs2015
	linux:  premake5.exe --os=linux gmake
]]

workspace "flatbuffers"
	configurations { "Debug", "Release" }
	targetdir "../bin/%{cfg.buildcfg}"
	includedirs {
		"../lua-intf",
	}
	flags {
		"C++11",
	}

	filter "configurations:Debug"
		flags { "Symbols" }
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	filter {}

project "flatbuffers"
	kind "SharedLib"
	files {
		"../flatbuffers.cpp",
	}
