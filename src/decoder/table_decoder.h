#ifndef LUA_FLATBUFFERS_DECODER_TABLE_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_TABLE_DECODER_H_

#include "decoder_base.h"

class TableDecoder final : public DecoderBase
{
public:
	explicit TableDecoder(DecoderContext& rCtx) : DecoderBase(rCtx) {};

	using Table = flatbuffers::Table;
	LuaRef DecodeTable(const reflection::Object& object, const Table& fbTable);

private:
	using Field = reflection::Field;

	void SplitFields(const reflection::Object& object);
	void DecodeScalarFields(const Table& fbTable);
	void DecodeNonScalarFields(const Table& fbTable);

	LuaRef DecodeNonScalarField(const Table& fbTable, const Field& field);
	LuaRef DecodeScalarField(const Table& fbTable, const Field& field);
	LuaRef DecodeStringField(const Table& fbTable, const Field& field);
	LuaRef DecodeVectorField(const Table& fbTable, const Field& field);
	LuaRef DecodeObjectField(const Table& fbTable, const Field& field);
	LuaRef DecodeUnionField(const Table& fbTable, const Field& field);

	LuaRef DecodeFieldBool(const Table& fbTable, const Field &field);

	template<typename T>
	LuaRef DecodeFieldI(const Table& fbTable, const Field &field);
	template<typename T>
	LuaRef DecodeFieldF(const Table& fbTable, const Field &field);

	template <typename T>
	bool VerifyFieldOfTable(const Table& fbTable, const Field &field);

private:
	using FieldVec = std::vector<const Field*>;
	FieldVec m_vScalarFields;
	FieldVec m_vNonScalarFields;

	LuaRef m_luaTable;
};  // class TableDecoder

#endif  // LUA_FLATBUFFERS_DECODER_TABLE_DECODER_H_
