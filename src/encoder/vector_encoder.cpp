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
	using namespace reflection;
	assert(elemType <= Double);
	assert(elemType != None);
	switch (elemType)
	{
	case UType:
	case Bool:
	case UByte:
		return CreateScalarVector<uint8_t>(luaArray);
	case Byte:
		return CreateScalarVector<int8_t>(luaArray);
	case Short:
		return CreateScalarVector<int16_t>(luaArray);
	case UShort:
		return CreateScalarVector<uint16_t>(luaArray);
	case Int:
		return CreateScalarVector<int32_t>(luaArray);
	case UInt:
		return CreateScalarVector<uint32_t>(luaArray);
	case Long:
		return CreateScalarVector<int64_t>(luaArray);
	case ULong:
		return CreateScalarVector<uint64_t>(luaArray);
	case Float:
		return CreateScalarVector<float>(luaArray);
	case Double:
		return CreateScalarVector<double>(luaArray);
	}  // switch
	assert(!"Illegal scalar element type.");
	return 0;
}  // CreateScalarVector()

uoffset_t VectorEncoder::EncodeStringVector(const LuaRef& luaArray)
{
	auto vStr = luaArray.toValue<std::vector<string>>();  // ?
	return Builder().CreateVectorOfStrings(vStr).o;
}

uoffset_t VectorEncoder::EncoderObjectVectort(
	const reflection::Object& obj, const LuaRef& luaArray)
{
	// XXX
	return 0;
}

template<typename T>
uoffset_t VectorEncoder::CreateScalarVector(const LuaRef& luaArray)
{
	auto v = luaArray.toValue<std::vector<T>>();  // ?
	return Builder().CreateVector(v).o;
}


