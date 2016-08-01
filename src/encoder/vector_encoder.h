#ifndef LUA_FLATBUFFERS_ENCODER_VECTOR_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_VECTOR_ENCODER_H_

#include "encoder_base.h"  // EncoderBase

class VectorEncoder final : EncoderBase
{
public:
	explicit VectorEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	flatbuffers::uoffset_t EncodeVector(
		const reflection::Type& type, const LuaRef& luaValue);

};  // class VectorEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_VECTOR_ENCODER_H_
