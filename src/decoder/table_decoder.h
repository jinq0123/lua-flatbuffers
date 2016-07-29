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
	LuaRef DecodeFieldOfTable(const Table& fbTable, const Field& field);
	LuaRef DecodeScalarField(const Table& fbTable, const Field& field);
	LuaRef DecodeStringField(const Table& fbTable, const Field& field);
	LuaRef DecodeVectorField(const Table& fbTable, const Field& field);
	LuaRef DecodeObjectField(const Table& fbTable, const Field& field);
	LuaRef DecodeUnionField(const Table& fbTable, const Field& field);

	template<typename T>
	LuaRef DecodeFieldI(const Table& fbTable, const Field &field);
	template<typename T>
	LuaRef DecodeFieldF(const Table& fbTable, const Field &field);

	template <typename T>
	bool VerifyFieldOfTable(const Table& fbTable, const Field &field);
};  // class TableDecoder

#endif  // LUA_FLATBUFFERS_DECODER_TABLE_DECODER_H_
