#ifndef LUA_FLATBUFFERS_ENCODER_VECTOR_ENCODER_H_
#define LUA_FLATBUFFERS_ENCODER_VECTOR_ENCODER_H_

#include "encoder_base.h"  // EncoderBase

class VectorEncoder final : EncoderBase
{
public:
	explicit VectorEncoder(EncoderContext& rCtx) : EncoderBase(rCtx) {};

public:
	using uoffset_t = flatbuffers::uoffset_t;
	uoffset_t EncodeVector(const reflection::Type& type, const LuaRef& luaValue);

private:
	using Object = reflection::Object;
	uoffset_t EncodeScalarVector(
		reflection::BaseType elemType,
		const LuaRef& luaArray);
	uoffset_t EncodeStringVector(const LuaRef& luaArray);
	uoffset_t EncoderObjectVector(const Object& elemObj, const LuaRef& luaArray);
	uoffset_t EncodeStructVector(const Object& elemObj, const LuaRef& luaArray);
	uoffset_t EncodeTableVector(const Object& elemObj, const LuaRef& luaArray);

	template<typename T>
	uoffset_t CreateScalarVector(const LuaRef& luaArray);
};  // class VectorEncoder

#endif  // LUA_FLATBUFFERS_ENCODER_VECTOR_ENCODER_H_
