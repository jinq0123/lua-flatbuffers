#include "vector_encoder.h"

// XXX: check luaArray is array

using flatbuffers::uoffset_t;

uoffset_t VectorEncoder::EncodeVector(
	const reflection::Type& type, const LuaRef& luaArray)
{
	using namespace reflection;
	assert(type.base_type() == Vector);
	BaseType elemType = type.element();
	if (elemType <= Double)
		return EncodeScalarVector(elemType, luaArray);
	if (elemType == String)
		return EncodeStringVector(luaArray);

	assert(elemType = Obj);
	const Object* pObj = Objects()[type.index()];
	assert(pObj);
	return EncoderObjectVectort(*pObj, luaArray);
}

uoffset_t VectorEncoder::EncodeScalarVector(
	reflection::BaseType elemType, const LuaRef& luaArray)
{
	// XXX
	return 0;
}

uoffset_t VectorEncoder::EncodeStringVector(const LuaRef& luaArray)
{
	// XXX
	return 0;
}

uoffset_t VectorEncoder::EncoderObjectVectort(
	const reflection::Object& obj, const LuaRef& luaArray)
{
	// XXX
	return 0;
}



