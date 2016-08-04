#include "vector_encoder.h"

#include "struct_encoder.h"  // for StructEncoder
#include "table_encoder.h"  // for TableEncoder

// XXX: check luaArray is array

using flatbuffers::uoffset_t;

uoffset_t VectorEncoder::EncodeVector(
	const reflection::Type& type, const LuaRef& luaArray)
{
	using namespace reflection;
	assert(type.base_type() == Vector);

	if (!luaArray.isTable())
	{
		SetError("array field " + PopFullName() + " is not array but "
			+ luaArray.typeName());
		return 0;
	}

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
	case Bool:  // true -> 1
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
	auto vStr = luaArray.toValue<std::vector<string>>();  // ? XXX
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

	assert(ILLEGAL_INDEX == m_idx);
	for (m_idx = 1; m_idx <= len; ++m_idx)
	{
		LuaRef luaElem = luaArray.get(m_idx);
		uint8_t* pDest = pBuf + (m_idx-1) * elemSize;
		// Todo: Push indexed name...
		StructEncoder(m_rCtx).EncodeStructToBuf(elemObj, luaElem, pDest);
		if (Bad()) return 0;
	}
	m_idx = ILLEGAL_INDEX;
	return offset;
}

uoffset_t VectorEncoder::EncodeTableVector(
	const Object& elemObj, const LuaRef& luaArray)
{
	assert(!elemObj.is_struct());
	assert(ILLEGAL_INDEX == m_idx);
	int len = luaArray.len();
	std::vector<uoffset_t> vOffsets;
	for (m_idx = 1; m_idx <= len; ++m_idx)
	{
		LuaRef luaElem = luaArray.get(m_idx);
		// Todo: Push indexed name...
		vOffsets.push_back(TableEncoder(m_rCtx).EncodeTable(elemObj, luaElem));
		if (Bad()) return 0;
	}
	m_idx = ILLEGAL_INDEX;

	// Todo: Sort...
	return Builder().CreateVector(vOffsets).o;
}

template<typename T>
uoffset_t VectorEncoder::CreateScalarVector(const LuaRef& luaArray)
{
	assert(ILLEGAL_INDEX == m_idx);
	std::vector<T> v;
	int len = luaArray.len();
	for (m_idx = 1; m_idx <= len; ++m_idx)
	{
		LuaRef luaElem = luaArray.get(m_idx);
		v.push_back(LuaToNumber<T>(luaElem));
		if (Bad()) return 0;
	}
	m_idx = ILLEGAL_INDEX;
	return Builder().CreateVector(v).o;
}

std::string VectorEncoder::GetVectorIndex() const
{
	if (ILLEGAL_INDEX == m_idx) return "";
	std::ostringstream oss;
	oss << "[" << m_idx << "]";
	return oss.str();
}

