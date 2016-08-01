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

public:
	const reflection::EnumVal* GetEnumVal(const LuaRef& luaType);
	int64_t GetType(const LuaRef& luaType);
	int64_t GetTypeFromName(const string& sType);

private:
	const reflection::Enum* m_pEnum;
};  // class UnionEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_UNION_ENCODER_H_
