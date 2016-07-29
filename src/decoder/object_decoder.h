#ifndef LUA_FLATBUFFERS_DECODER_OBJECT_DECODER_H_
#define LUA_FLATBUFFERS_DECODER_OBJECT_DECODER_H_

#include "decoder_base.h"

class ObjectDecoder final : public DecoderBase
{
public:
	explicit ObjectDecoder(DecoderContext& rCtx) : DecoderBase(rCtx) {};

public:
	LuaRef DecodeObject(const reflection::Object& object, const void* pData);
};  // class ObjectDecoder

#endif  // LUA_FLATBUFFERS_DECODER_OBJECT_DECODER_H_
