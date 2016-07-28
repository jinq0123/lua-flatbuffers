#ifndef LUA_FLATBUFFERS_OBJECT_DECODER_H_
#define LUA_FLATBUFFERS_OBJECT_DECODER_H_

#include "decoder_base.h"

class ObjectDecoder final : public DecoderBase
{
public:
	explicit ObjectDecoder(DecoderContext& rCtx);

public:
	LuaRef DecodeObject(const reflection::Object& object, const void* pData);
};  // class Decoder

#endif  // LUA_FLATBUFFERS_OBJECT_DECODER_H_
