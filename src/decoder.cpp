#include "decoder.h"

#include <LuaIntf/LuaIntf.h>

Decoder::Decoder(lua_State* state, const reflection::Schema& schema)
	: L(state), m_schema(schema)
{
	assert(L);
}

std::tuple<LuaIntf::LuaRef, std::string>
Decoder::Decode(const std::string& sName, const std::string& buf)
{
	return std::make_tuple(LuaIntf::LuaRef(L, nullptr), "to be implemented");
}

