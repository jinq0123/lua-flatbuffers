#ifndef LUA_FLATBUFFERS_DECODER_STRUCT_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_STRUCT_DECODER_H_

#include "decoder_base.h"

#include <flatbuffers/flatbuffers.h>

namespace reflection {
enum BaseType;
struct Field;
struct Object;
struct Type;
}

class StructDecoder final : public DecoderBase
{
public:
	explicit StructDecoder(DecoderContext& rCtx);

public:
	using Struct = flatbuffers::Struct;
	LuaRef DecodeStruct(const reflection::Object& object,
		const flatbuffers::Struct& fbStruct);

private:
	LuaRef DecodeFieldOfStruct(const Struct& fbStruct,
		const reflection::Field& field);
	LuaRef DecodeScalarField(const Struct& fbStruct,
		const reflection::Field& field);
	LuaRef DecodeStringField(const Struct& fbStruct,
		const reflection::Field& field);
	LuaRef DecodeVectorField(const Struct& fbStruct,
		const reflection::Field& field);
	LuaRef DecodeObjectField(const Struct& fbStruct,
		const reflection::Field& field);
	LuaRef DecodeUnionField(const Struct& fbStruct,
		const reflection::Field& field);

	template<typename T>
	LuaRef DecodeFieldI(const Struct& fbStruct, const reflection::Field &field);
	template<typename T>
	LuaRef DecodeFieldF(const Struct& fbStruct, const reflection::Field &field);
};  // class StructDecoder

#endif  // LUA_FLATBUFFERS_DECODER_STRUCT_DECODER_H_
