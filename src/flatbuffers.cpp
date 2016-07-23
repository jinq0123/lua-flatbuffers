// FlatBuffers library for Lua.
// Author: Jin Qing ( http://blog.csdn.net/jq0123 )

#include "schema_cache.h"  // for SchemaCache

#include "encoder.h"

// By default LuaIntf expect the Lua library to build under C++.
// If you really want to use Lua library compiled under C,
// you can define LUAINTF_LINK_LUA_COMPILED_IN_CXX to 0:
// See: https://github.com/SteveKChiu/lua-intf
// #define LUAINTF_LINK_LUA_COMPILED_IN_CXX 0
#include <LuaIntf/LuaIntf.h>

#include <iostream>

namespace {

void test()
{
	std::cout << "test...\n";
}

SchemaCache& GetCache()
{
	static SchemaCache s_cache;
	return s_cache;
}

std::tuple<bool, std::string> LoadBfbsFile(const std::string& sBfbsFile)
{
	return GetCache().LoadBfbsFile(sBfbsFile);
}

std::tuple<bool, std::string> LoadBfbs(const std::string& sBfbs)
{
	return GetCache().LoadBfbs(sBfbs);
}

std::tuple<bool, std::string> LoadFbsFile(const std::string& sFbsFile)
{
	return GetCache().LoadFbsFile(sFbsFile);
}

std::tuple<bool, std::string> LoadFbs(const std::string& sFbs)
{
	return GetCache().LoadFbs(sFbs);
}

// Encode lua table to buffer.
// Returns (buffer, "") or (nil, error)
std::tuple<LuaIntf::LuaRef, std::string> Encode(
	const std::string& sName, const LuaIntf::LuaRef& table)
{
	lua_State* luaState = table.state();
	const reflection::Schema* pSchema = GetCache().GetSchemaOfObject(sName);
	if (!pSchema)
	{
		return std::make_tuple(
			LuaIntf::LuaRef(luaState, nullptr),
			"no type " + sName);
	}

	Encoder encoder(*pSchema);
	if (encoder.Encode(sName, table))
	{
		return std::make_tuple(LuaIntf::LuaRef::fromValue(
			luaState, encoder.GetResultStr()), "");
	}
	return std::make_tuple(
		LuaIntf::LuaRef(luaState, nullptr),
		encoder.GetErrorStr());
}

// Decode buffer to lua table.
// Returns (table, "") or (nil, error)
std::tuple<LuaIntf::LuaRef, std::string> Decode(
	lua_State* L,
	const std::string& sName, const std::string& buf)
{
	assert(L);
	return std::make_tuple(LuaIntf::LuaRef(L, nullptr), "to be implemented");
}

}  // namespace

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
			return Decode(L, sName, buf);
		});
	mod.pushToStack();
	return 1;
}
