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

	--[[
	From: https://github.com/SteveKChiu/lua-intf
	By default LuaIntf expect the Lua library to build under C++.
	If you really want to use Lua library compiled under C,
	you can define LUAINTF_LINK_LUA_COMPILED_IN_CXX to 0:
	--]]
	-- defines { "LUAINTF_LINK_LUA_COMPILED_IN_CXX=0" }

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
