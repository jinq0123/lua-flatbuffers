// FlatBuffers library for Lua.
// Author: Jin Qing ( http://blog.csdn.net/jq0123 )

// By default LuaIntf expect the Lua library to build under C++.
// If you really want to use Lua library compiled under C,
// you can define LUAINTF_LINK_LUA_COMPILED_IN_CXX to 0:
// See: https://github.com/SteveKChiu/lua-intf

// #define LUAINTF_LINK_LUA_COMPILED_IN_CXX 0
#include "LuaIntf/LuaIntf.h"

#include <iostream>

static void test()
{
	std::cout << "test...\n";
}

static std::tuple<bool, std::string> LoadBfbsFile(const std::string& sBfbsFile)
{
	return std::make_tuple(false, "To be implemented");
}

static std::tuple<bool, std::string> LoadBfbs(const std::string& sBfbs)
{
	return std::make_tuple(false, "To be implemented");
}

static std::tuple<bool, std::string> LoadFbsFile(const std::string& sFbsFile)
{
	return std::make_tuple(false, "To be implemented");
}

static std::tuple<bool, std::string> LoadFbs(const std::string& sFbs)
{
	return std::make_tuple(false, "To be implemented");
}

// Encode lua table to buffer.
// Returns (true, buffer) or (false, error)
static std::tuple<bool, std::string> Encode(
	const std::string& sName, const LuaIntf::LuaRef& table)
{
	return std::make_tuple(false, "To be implemented");
}

// Decode buffer to lua table.
// Returns (table, "") or (nil, error)
static std::tuple<LuaIntf::LuaRef, std::string> Decode(
	lua_State* L,
	const std::string& sName, const std::string& buf)
{
	assert(L);
	return std::make_tuple(LuaIntf::LuaRef(L, nullptr), "To be implemented");
}

extern "C"
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__CODEGEARC__)
__declspec(dllexport)
#endif
int luaopen_flatbuffers(lua_State* L)
{
	using namespace LuaIntf;
	LuaRef mod = LuaRef::createTable(L);
	LuaBinding(mod)
		.addFunction("test", &test)
		.addFunction("load_bfbs_file", &LoadBfbsFile)
		.addFunction("load_bfbs", &LoadBfbs)
		.addFunction("load_fbs_file", &LoadFbsFile)
		.addFunction("load_fbs", &LoadFbs)
		.addFunction("encode", &Encode)
		.addFunction("decode", [L](const std::string& sName,
			const std::string& buf) {
			Decode(L, sName, buf);
		});
	mod.pushToStack();
	return 1;
}
