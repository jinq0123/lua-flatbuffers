#ifndef LUA_FLATBUFFERS_ENCODER_UNION_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_UNION_ENCODER_H_

#include "encoder_base.h"  // EncoderBase

class UnionEncoder final : EncoderBase
{
public:
	explicit UnionEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	flatbuffers::uoffset_t EncodeUnion(
		const reflection::Enum& enu,
		const LuaRef& luaType,
		const LuaRef& luaValue);
};  // class UnionEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_UNION_ENCODER_H_
