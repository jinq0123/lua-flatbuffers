#ifndef LUA_FLATBUFFERS_ENCODER_UNION_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_UNION_ENCODER_H_

#include "encoder_base.h"  // EncoderBase

class UnionEncoder final : EncoderBase
{
public:
	explicit UnionEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	using Enum = reflection::Enum;
	flatbuffers::uoffset_t EncodeUnion(
		const Enum& enu, const LuaRef& luaType, const LuaRef& luaValue);

private:
	using EnumVal = reflection::EnumVal;
	const EnumVal* GetEnumVal(const Enum& enu, const LuaRef& luaType);
	const EnumVal* GetEnumValFromNum(const Enum& enu, int64_t qwType);
	const EnumVal* GetEnumValFromName(const Enum& enu, const string& sType);
};  // class UnionEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_UNION_ENCODER_H_
