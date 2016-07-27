#ifndef LUA_FLATBUFFERS_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_H_

#include "name_stack.h"

#include <flatbuffers/flatbuffers.h>

namespace LuaIntf {
class LuaRef;
}

namespace reflection {
struct Enum;
struct Field;
struct Object;
struct Schema;
struct Type;
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
		const std::string& sName, const std::string& buf);

private:
	using Table = flatbuffers::Table;
	LuaRef DecodeFieldOfTable(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeScalarField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeStringField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeVectorField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeObjectField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeUnionField(const Table& table,
		const reflection::Field& field);

	template<typename T>
	LuaRef DecodeFieldI(const Table& fbTable, const reflection::Field &field);
	template<typename T>
	LuaRef DecodeFieldF(const Table& fbTable, const reflection::Field &field);

	LuaRef DecodeObject(
		const reflection::Object& object,
		const Table& fbTable);
	LuaRef DecodeVector(const reflection::Type& type,
		const flatbuffers::VectorOfAny& v);

	LuaRef Decode(const reflection::Type& type,
		const void* pVoid);

private:
	template <typename T>
	bool VerifyFieldOfTable(const Table& fbTable,
		const reflection::Field &field);

private:
	bool Bad() const { return !m_sError.empty(); }
	LuaRef Nil() const;
	void SetError(const std::string& sError);
	std::string PopFullName();
	std::string PopFullFieldName(const std::string& sFieldName);

private:
	lua_State* L;
	const reflection::Schema& m_schema;
	const flatbuffers::Vector<flatbuffers::Offset<
		reflection::Object>>& m_vObjects;
	const flatbuffers::Vector<flatbuffers::Offset<
		reflection::Enum>>& m_vEnums;

	std::unique_ptr<flatbuffers::Verifier> m_pVerifier;
	std::string m_sError;
	NameStack m_nameStack;  // For error message.
};  // class Decoder

#endif  // LUA_FLATBUFFERS_DECODER_H_
