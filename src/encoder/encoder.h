#ifndef LUA_FLATBUFFERS_ENCODER_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_ENCODER_H_

#include "encoder_base.h"

class Encoder final : public EncoderBase
{
public:
	explicit Encoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	bool Encode(const string& sName, const LuaRef& luaTable);
	string GetResultStr() const;

public:
	static LuaRef TestToNum(const LuaRef& luaVal);  // for test
};  // class Encoder

#endif  // LUA_FLATBUFFERS_ENCODER_ENCODER_H_
