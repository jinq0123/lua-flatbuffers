#ifndef LUA_FLATBUFFERS_ROOT_DECODER_H_
#define LUA_FLATBUFFERS_ROOT_DECODER_H_

#include "decoder_base.h"

class RootDecoder final : public DecoderBase
{
public:
	RootDecoder(DecoderContext& rCtx);

public:
	// Decode buffer to lua table.
	LuaRef Decode(const std::string& sName, const void* pBuf);
};  // class RootDecoder

#endif  // LUA_FLATBUFFERS_ROOT_DECODER_H_
