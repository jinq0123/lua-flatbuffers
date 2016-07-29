#ifndef LUA_FLATBUFFERS_DECODER_UNION_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_UNION_DECODER_H_

#include "decoder_base.h"

class UnionDecoder final : public DecoderBase
{
public:
	explicit UnionDecoder(DecoderContext& rCtx);

	LuaRef DecodeUnion(const reflection::Type& type, const void* pData);

private:
	template <typename T>
	inline LuaRef DecodeScalar(const void* pData);
};  // class UnionDecoder

#endif  // LUA_FLATBUFFERS_DECODER_UNION_DECODER_H_
