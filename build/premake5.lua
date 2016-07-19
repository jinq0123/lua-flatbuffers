-- premake5.lua
--[[
Usage: 
	windows: premake5.exe --os=windows vs2015
	linux:  premake5.exe --os=linux gmake
]]

local lua_include_dir = "../lua532/include"
local lua_lib_dir = "../lua532/lib"

workspace "flatbuffers"
	configurations { "Debug", "Release" }

project "flatbuffers"
	kind "SharedLib"
	targetdir "../bin/%{cfg.buildcfg}"

	files {
		"../src/**.h",
		"../src/**.cpp",
	}
	includedirs {
		"../lua-intf",
		lua_include_dir,
	}
	libdirs {
		lua_lib_dir,
	}
	flags {
		"C++11",
	}

	filter "configurations:Debug"
		flags { "Symbols" }
		libdirs { lua_lib_dir .. "/Debug" }
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		libdirs { lua_lib_dir .. "/Release" }
	filter {}

	links {
		"lua",
	}
