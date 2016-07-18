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

extern "C" int luaopen_flatbuffers(lua_State* L)
{
	using namespace LuaIntf;
	LuaRef mod = LuaRef::createTable(L);
	LuaBinding(mod).beginModule("ttt")
		.addFunction("test", &test)
	.endModule();
	mod.pushToStack();
	return 1;
}
