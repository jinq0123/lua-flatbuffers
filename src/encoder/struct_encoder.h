#ifndef LUA_FLATBUFFERS_ENCODER_STRUCT_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_STRUCT_ENCODER_H_

#include "encoder_base.h"  // EncoderBase

class StructEncoder final : EncoderBase
{
public:
	explicit StructEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	// Encode recursively. Return 0 and set sError if any error.
	using Object = reflection::Object;
	flatbuffers::uoffset_t EncodeStruct(const Object& obj, const LuaRef& luaTable);

private:
	using Field = reflection::Field;

	void CheckLuaTable(const Object& obj, const LuaRef& luaTable);
	void EncodeStructToBuf(const Object& obj,
		const LuaRef& luaTable, uint8_t* pBuf);
	void EncodeStructFieldToBuf(const Field& field,
		const LuaRef& luaTable, uint8_t* pBuf);
	void EncodeScalarToBuf(reflection::BaseType eType,
		const LuaRef& luaValue, uint8_t* pDest);
};  // class StructEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_STRUCT_ENCODER_H_
