-- premake5.lua
--[[
Usage: 
	windows: premake5.exe --os=windows vs2015
	linux:  premake5.exe --os=linux gmake
]]

local lua_include_dir = "../third_party/lua532/include"
local lib_dir = "../third_party/lib"

workspace "lfb"
	configurations { "Debug", "Release" }

project "lfb"
	kind "SharedLib"
	targetdir "../bin/%{cfg.buildcfg}"

	files {
		"../src/**.h",
		"../src/**.cpp",
	}
	includedirs {
		"../third_party/lua-intf",
		"../third_party/flatbuffers/include",
		lua_include_dir,
	}
	libdirs {
		lib_dir,
	}
	flags {
		"C++11",
	}

	filter "configurations:Debug"
		flags { "Symbols" }
		libdirs { lib_dir .. "/Debug" }
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		libdirs { lib_dir .. "/Release" }
	filter {}

	links {
		"lua",
		"flatbuffers"
	}
