#include "vector_encoder.h"

#include "struct_encoder.h"  // for StructEncoder
#include "table_encoder.h"  // for TableEncoder

// XXX: check luaArray is array

using flatbuffers::uoffset_t;

uoffset_t VectorEncoder::EncodeVector(
	const reflection::Type& type, const LuaRef& luaArray)
{
	using namespace reflection;
	assert(luaArray.isTable());  // Todo: check array...
	assert(type.base_type() == Vector);
	BaseType elemType = type.element();
	if (elemType <= Double)
		return EncodeScalarVector(elemType, luaArray);
	if (elemType == String)
		return EncodeStringVector(luaArray);

	assert(elemType = Obj);
	const Object* pObj = Objects()[type.index()];
	assert(pObj);
	return EncoderObjectVector(*pObj, luaArray);
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

uoffset_t VectorEncoder::EncoderObjectVector(
	const reflection::Object& elemObj, const LuaRef& luaArray)
{
	if (elemObj.is_struct())
		return EncodeStructVector(elemObj, luaArray);
	return EncodeTableVector(elemObj, luaArray);
}

uoffset_t VectorEncoder::EncodeStructVector(
	const Object& elemObj, const LuaRef& luaArray)
{
	assert(elemObj.is_struct());
	int len = luaArray.len();
	uint8_t* pBuf = 0;
	int32_t elemSize = elemObj.bytesize();
	uoffset_t offset = Builder()
		.CreateUninitializedVector(len, elemSize, &pBuf);

	for (int i = 1; i <= len; ++i)
	{
		LuaRef luaElem = luaArray.get(i);
		uint8_t* pDest = pBuf + (i-1) * elemSize;
		// Todo: Push indexed name...
		StructEncoder(m_rCtx).EncodeStructToBuf(elemObj, luaElem, pDest);
		if (Bad()) return 0;
	}
	return offset;
}

uoffset_t VectorEncoder::EncodeTableVector(
	const Object& elemObj, const LuaRef& luaArray)
{
	assert(!elemObj.is_struct());
	int len = luaArray.len();
	std::vector<uoffset_t> vOffsets;
	for (int i = 1; i <= len; ++i)
	{
		LuaRef luaElem = luaArray.get(i);
		// Todo: Push indexed name...
		vOffsets.push_back(TableEncoder(m_rCtx).EncodeTable(elemObj, luaElem));
		if (Bad()) return 0;
	}

	// Todo: Sort...
	return Builder().CreateVector(vOffsets).o;
}

template<typename T>
uoffset_t VectorEncoder::CreateScalarVector(const LuaRef& luaArray)
{
	auto v = luaArray.toValue<std::vector<T>>();  // ?
	return Builder().CreateVector(v).o;
}


