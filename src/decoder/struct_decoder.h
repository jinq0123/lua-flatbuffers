#ifndef LUA_FLATBUFFERS_DECODER_STRUCT_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_STRUCT_DECODER_H_

#include "decoder_base.h"

class StructDecoder final : public DecoderBase
{
public:
	explicit StructDecoder(DecoderContext& rCtx);

public:
	using Struct = flatbuffers::Struct;
	LuaRef DecodeStruct(const reflection::Object& object,
		const flatbuffers::Struct& fbStruct);

private:
	using Field = reflection::Field;
	LuaRef DecodeFieldOfStruct(const Struct& fbStruct, const Field& field);
	LuaRef DecodeScalarField(const Struct& fbStruct, const Field& field);
	LuaRef DecodeObjectField(const Struct& fbStruct, const Field& field);
};  // class StructDecoder

#endif  // LUA_FLATBUFFERS_DECODER_STRUCT_DECODER_H_
