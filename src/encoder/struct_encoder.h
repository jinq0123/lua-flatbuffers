#ifndef LUA_FLATBUFFERS_ENCODER_STRUCT_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_STRUCT_ENCODER_H_

#include "encoder_base.h"  // EncoderBase

#include <unordered_map>

class StructEncoder final : EncoderBase
{
public:
	explicit StructEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	// Encode recursively. Return 0 and set m_sError if any error.
	using Object = reflection::Object;
	using uoffset_t = flatbuffers::uoffset_t;
	using Field = reflection::Field;

	uoffset_t EncodeStruct(const Object& obj, const LuaRef& luaTable);

private:
	bool CheckStructFields(const Object& obj, const LuaRef& luaTable);
	bool EncodeStructToBuf(const Object& obj,
		const LuaRef& luaTable, uint8_t* pBuf);
	bool EncodeStructFieldToBuf(const Field& field,
		const LuaRef& luaTable, uint8_t* pBuf);
	void EncodeStructElementToBuf(reflection::BaseType eType,
		const LuaRef& luaValue, uint8_t* pDest);
};  // class StructEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_STRUCT_ENCODER_H_
