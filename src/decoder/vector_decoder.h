#ifndef LUA_FLATBUFFERS_DECODER_VECTOR_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_VECTOR_DECODER_H_

#include "decoder_base.h"

class VectorDecoder final : public DecoderBase
{
public:
	explicit VectorDecoder(DecoderContext& rCtx);

	using VectorOfAny = flatbuffers::VectorOfAny;
	LuaRef DecodeVector(const reflection::Type& type, const VectorOfAny& v);

private:
	LuaRef DecodeScalarVector(reflection::BaseType elemType,
		const VectorOfAny& v);
	LuaRef DecodeStringVector(const VectorOfAny& v);
	LuaRef DecodeObjVector(const reflection::Object& elemObj,
		const VectorOfAny& v);
	LuaRef DecodeStructVector(const reflection::Object& elemObj,
		const VectorOfAny& v);
	LuaRef DecodeTableVector(const reflection::Object& elemObj,
		const VectorOfAny& v);

	LuaRef DecodeUnion(const reflection::Type& type, const void* pData);
};  // class VectorDecoder

#endif  // LUA_FLATBUFFERS_DECODER_VECTOR_DECODER_H_
