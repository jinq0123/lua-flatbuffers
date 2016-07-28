#ifndef LUA_FLATBUFFERS_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_H_

#include "name_stack.h"

#include <flatbuffers/flatbuffers.h>

namespace LuaIntf {
class LuaRef;
}

namespace reflection {
enum BaseType;
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
	using Struct = flatbuffers::Struct;
	LuaRef DecodeFieldOfTable(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeFieldOfStruct(const Struct& fbStruct,
		const reflection::Field& field);
	LuaRef DecodeScalarField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeStringField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeVectorField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeObjectField(const Table& fbTable,
		const reflection::Field& field);
	LuaRef DecodeUnionField(const Table& fbTable,
		const reflection::Field& field);

	template<typename T>
	LuaRef DecodeFieldI(const Table& fbTable, const reflection::Field &field);
	template<typename T>
	LuaRef DecodeFieldF(const Table& fbTable, const reflection::Field &field);

	LuaRef DecodeObject(const reflection::Object& object, const void* pData);
	LuaRef DecodeTable(const reflection::Object& object, const Table& fbTable);
	LuaRef DecodeStruct(const reflection::Object& object,
		const flatbuffers::Struct& fbStruct);

	LuaRef DecodeVector(const reflection::Type& type,
		const flatbuffers::VectorOfAny& v);
	LuaRef DecodeScalarVector(reflection::BaseType elemType,
		const flatbuffers::VectorOfAny& v);
	LuaRef DecodeStringVector(const flatbuffers::VectorOfAny& v);
	LuaRef DecodeObjVector(const reflection::Object& elemObj,
		const flatbuffers::VectorOfAny& v);
	LuaRef DecodeStructVector(const reflection::Object& elemObj,
		const flatbuffers::VectorOfAny& v);
	LuaRef DecodeTableVector(const reflection::Object& elemObj,
		const flatbuffers::VectorOfAny& v);

	template <typename T>
	inline LuaRef Decoder::DecodeScalar(const void* pData);

	LuaRef DecodeUnion(const reflection::Type& type, const void* pData);

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
	std::string PopFullVectorName(size_t index);

private:
	lua_State* L;
	const reflection::Schema& m_schema;
	const flatbuffers::Vector<flatbuffers::Offset<
		reflection::Object>>& m_vObjects;

	std::unique_ptr<flatbuffers::Verifier> m_pVerifier;
	std::string m_sError;
	NameStack m_nameStack;  // For error message.
};  // class Decoder

#endif  // LUA_FLATBUFFERS_DECODER_H_
