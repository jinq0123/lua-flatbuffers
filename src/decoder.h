#ifndef LUA_FLATBUFFERS_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_H_

#include <flatbuffers/flatbuffers.h>

namespace LuaIntf {
class LuaRef;
}

namespace reflection {
struct Field;
struct Object;
struct Schema;
}

struct lua_State;

class Decoder final
{
public:
	Decoder(lua_State* state, const reflection::Schema& schema);

public:
	using LuaRef = LuaIntf::LuaRef;

	// Decode buffer to lua table.
	// Returns (table, "") or (nil, error)
	std::tuple<LuaRef, std::string> Decode(
		const std::string& sName, const std::string& buf) const;

private:
	using Table = flatbuffers::Table;
	void SetLuaTableField(
		const Table& fbTable,
		const reflection::Field& field,
		LuaRef& rLuaTable) const;

	LuaRef DecodeObject(
		const reflection::Object& object,
		const Table& fbTable) const;
	LuaRef DecodeVectorField(
		const Table& table,
		const reflection::Field& field) const;

private:
	lua_State* L;
	const reflection::Schema& m_schema;
	const flatbuffers::Vector<flatbuffers::Offset<
		reflection::Object>>& m_vObjects;
};  // class Decoder

#endif  // LUA_FLATBUFFERS_DECODER_H_
