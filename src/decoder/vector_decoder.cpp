#include "vector_decoder.h"

#include "struct_decoder.h"  // for StructDecoder
#include "table_decoder.h"  // for TableDecoder

using LuaIntf::LuaRef;

LuaRef VectorDecoder::DecodeVector(
	const reflection::Type& type, const VectorOfAny& v)
{
	assert(reflection::Vector == type.base_type());
	reflection::BaseType elemType = type.element();
	assert(reflection::Vector != elemType && "Nesting vectors is not supported.");
	assert(reflection::Union != elemType && "Union must always be part of a table.");

	int32_t iTypeIndex = type.index();
	size_t uElemSize = flatbuffers::GetTypeSizeInline(
		elemType, iTypeIndex, m_rCtx.schema);
	const uint8_t* end;
	if (!Verifier().VerifyVector(reinterpret_cast<
		const uint8_t*>(&v), uElemSize, &end))
		ERR_RET_NIL("illegal vector " + PopFullName());

	// Todo: Check key order.

	if (elemType <= reflection::Double)
		return DecodeScalarVector(elemType, v);
	if (reflection::String == elemType)
		return DecodeStringVector(v);
	if (reflection::Obj == elemType)
		return DecodeObjVector(*Objects()[iTypeIndex], v);
	assert(!"Illegal element type.");
	return Nil();
}

LuaRef VectorDecoder::DecodeScalarVector(
	reflection::BaseType elemType, const VectorOfAny& v)
{
	assert(elemType <= reflection::Double);

	LuaRef luaArray = CreateLuaTable();
	const VectorOfAny* pVec = &v;
	if (elemType <= reflection::Long)
	{
		for (size_t i = 0; i < v.size(); ++i)
			luaArray[i+1] = GetAnyVectorElemI(pVec, elemType, i);
	}
	else if (elemType == reflection::ULong)
	{
		for (size_t i = 0; i < v.size(); ++i)
			luaArray[i+1] = static_cast<uint64_t>(
				GetAnyVectorElemI(pVec, elemType, i));
	}
	else
	{
		for (size_t i = 0; i < v.size(); ++i)
			luaArray[i+1] = GetAnyVectorElemF(pVec, elemType, i);
	}
	return luaArray;
}

LuaRef VectorDecoder::DecodeStringVector(const VectorOfAny& v)
{
	LuaRef luaArray = CreateLuaTable();
	for (size_t i = 0; i < v.size(); ++i)
	{
		const auto* pStr = flatbuffers::GetAnyVectorElemPointer<
			const flatbuffers::String>(&v, i);
		if (!Verifier().Verify(pStr))
			ERR_RET_NIL("illegal string vector item " + PopFullVectorName(i));
		luaArray[i+1] = pStr->str();
	}
	return luaArray;
}

LuaRef VectorDecoder::DecodeObjVector(
	const reflection::Object& elemObj, const VectorOfAny& v)
{
	if (elemObj.is_struct())
		return DecodeStructVector(elemObj, v);
	return DecodeTableVector(elemObj, v);
}

LuaRef VectorDecoder::DecodeStructVector(
	const reflection::Object& elemObj, const VectorOfAny& v)
{
	assert(elemObj.is_struct());

	LuaRef luaArray = CreateLuaTable();
	for (size_t i = 0; i < v.size(); ++i)
	{
		const auto* pStruct = flatbuffers::GetAnyVectorElemAddressOf<
			const flatbuffers::Struct>(&v, i, elemObj.bytesize());
		assert(pStruct);
		luaArray[i+1] = StructDecoder(m_rCtx).DecodeStruct(elemObj, *pStruct);
		if (Bad()) return Nil();
	}
	return luaArray;
}

LuaRef VectorDecoder::DecodeTableVector(
	const reflection::Object& elemObj, const VectorOfAny& v)
{
	assert(!elemObj.is_struct());

	LuaRef luaArray = CreateLuaTable();
	for (size_t i = 0; i < v.size(); ++i)
	{
		const auto* pTable = flatbuffers::GetAnyVectorElemPointer<
			const flatbuffers::Table>(&v, i);
		if (!pTable)
			ERR_RET_NIL("illegal vector element point " + PopFullVectorName(i));
		luaArray[i+1] = TableDecoder(m_rCtx).DecodeTable(elemObj, *pTable);
		if (Bad()) return Nil();
	}
	return luaArray;
}
