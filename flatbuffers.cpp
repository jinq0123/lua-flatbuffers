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

// FlatBuffers Schema
class Schema
{
public:
	explicit Schema(const std::string& sBfbsFilePath);
	~Schema();

public:
	void Test() { std::cout << "Test\n"; }

public:
	void LoadFromBfbs(const std::string& sBfbs);
	void LoadFromFile(const std::string& sFilePath);
};  // class Schema

Schema::Schema(const std::string& sBfbsFilePath)
{
	LoadFromFile(sBfbsFilePath);

	if (sBfbsFilePath.empty())
		std::cout << "empty path!\n";
	else
		std::cout << sBfbsFilePath << "\n";
}

Schema::~Schema()
{
	std::cout << "~Schema()\n";
}

void Schema::LoadFromBfbs(const std::string& sBfbs)
{
	// XXX
}

void Schema::LoadFromFile(const std::string& sFilePath)
{
	// XXX
}

static std::shared_ptr<Schema> GetSchemaPtr()
{
	return std::make_shared<Schema>("Test Sp");
}

namespace LuaIntf
{
	LUA_USING_SHARED_PTR_TYPE(std::shared_ptr)
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
		.addFunction("get_schema_ptr", GetSchemaPtr)
		.beginClass<Schema>("Schema")
			.addConstructor(LUA_ARGS(_opt<std::string>))
			.addFunction("test", &Schema::Test)
		.endClass();
	mod.pushToStack();
	return 1;
}
