#ifndef LUA_FLATBUFFERS_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_H_

#include "decoder_base.h"

#include <flatbuffers/flatbuffers.h>

namespace reflection {
enum BaseType;
struct Field;
struct Object;
struct Type;
}

class Decoder final : public DecoderBase
{
public:
	explicit Decoder(DecoderContext& rCtx);

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
	inline LuaRef DecodeScalar(const void* pData);

	LuaRef DecodeUnion(const reflection::Type& type, const void* pData);

private:
	template <typename T>
	bool VerifyFieldOfTable(const Table& fbTable,
		const reflection::Field &field);
};  // class Decoder

#endif  // LUA_FLATBUFFERS_DECODER_H_
