#ifndef LUA_FLATBUFFERS_DECODER_UNION_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_UNION_DECODER_H_

#include "decoder_base.h"

class UnionDecoder final : public DecoderBase
{
public:
	explicit UnionDecoder(DecoderContext& rCtx) : DecoderBase(rCtx) {};

	LuaRef DecodeUnion(const reflection::Type& type,
		int64_t nUnionType, const void* pData);
};  // class UnionDecoder

#endif  // LUA_FLATBUFFERS_DECODER_UNION_DECODER_H_
