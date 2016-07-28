#include "decoder.h"

#include <flatbuffers/reflection.h>
#include <LuaIntf/LuaIntf.h>

using LuaIntf::LuaRef;

Decoder::Decoder(DecoderContext& rCtx) : DecoderBase(rCtx)
{
}

LuaRef Decoder::DecodeFieldOfTable(
	const Table& fbTable, const reflection::Field& field)
{
	if (field.deprecated()) return Nil();
	reflection::BaseType eType = field.type()->base_type();
	if (eType <= reflection::Double)
		return DecodeScalarField(fbTable, field);

	if (!VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field))
		return Nil();

	switch (eType)
	{
	case reflection::String:
		return DecodeStringField(fbTable, field);
	case reflection::Vector:
		return DecodeVectorField(fbTable, field);
	case reflection::Obj:
		return DecodeObjectField(fbTable, field);
	case reflection::Union:
		return DecodeUnionField(fbTable, field);
	}
	assert(!"Illegal field type.");
	return Nil();
}

LuaRef Decoder::DecodeFieldOfStruct(const Struct& fbStruct,
	const reflection::Field& field)
{
	// XXX
	return Nil();
}

LuaRef Decoder::DecodeScalarField(
	const Table& fbTable, const reflection::Field& field)
{
	switch (field.type()->base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		return DecodeFieldI<uint8_t>(fbTable, field);
	case reflection::Byte:
		return DecodeFieldI<int8_t>(fbTable, field);
	case reflection::Short:
		return DecodeFieldI<int16_t>(fbTable, field);
	case reflection::UShort:
		return DecodeFieldI<uint16_t>(fbTable, field);
	case reflection::Int:
		return DecodeFieldI<int32_t>(fbTable, field);
	case reflection::UInt:
		return DecodeFieldI<uint32_t>(fbTable, field);
	case reflection::Long:
		return DecodeFieldI<int64_t>(fbTable, field);
	case reflection::ULong:
		return DecodeFieldI<uint64_t>(fbTable, field);
	case reflection::Float:
		return DecodeFieldF<float>(fbTable, field);
	case reflection::Double:
		return DecodeFieldF<double>(fbTable, field);
	}
	assert(!"Illegal scalar field type.");
	return Nil();
}

LuaRef Decoder::DecodeStringField(
	const Table& fbTable, const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const flatbuffers::String* pStr = flatbuffers::GetFieldS(fbTable, field);
	if (!m_pVerifier->Verify(pStr))
	{
		ERR_RET_NIL("illegal string field "
			+ PopFullFieldName(field.name()->c_str()));
	}
	if (pStr) return LuaRef::fromValue(L, pStr->str());
	return Nil();
}

LuaRef Decoder::DecodeVectorField(
	const Table& fbTable,
	const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const auto* pVec = fbTable.GetPointer<
		const flatbuffers::VectorOfAny*>(field.offset());
	if (!pVec) return Nil();

	const reflection::Type& type = *field.type();
	m_nameStack.Push(field.name()->c_str());
	LuaRef luaTable = DecodeVector(type, *pVec);
	m_nameStack.SafePop();
	return luaTable;
}

LuaRef Decoder::DecodeObjectField(
	const Table& fbTable, const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const void* pData = fbTable.GetPointer<const void*>(field.offset());
	return DecodeObject(*m_vObjects[field.type()->index()], pData);
}

LuaRef Decoder::DecodeUnionField(const Table& fbTable,
	const reflection::Field& field)
{
	assert(VerifyFieldOfTable<flatbuffers::uoffset_t>(fbTable, field));
	const reflection::Type& type = *field.type();
	assert(type.base_type() == reflection::Union);
	const void* pData = fbTable.GetPointer<const void*>(field.offset());
	if (!pData) return Nil();

	const reflection::Enum& e = *(*m_schema.enums())[type.index()];
	assert(e.is_union());
	const reflection::Type& underlyingType = *e.underlying_type();
	m_nameStack.Push(field.name()->c_str());
	LuaRef luaRef = DecodeUnion(underlyingType, pData);
	m_nameStack.SafePop();
	return luaRef;
}

template<typename T>
LuaRef Decoder::DecodeFieldI(const Table& fbTable,
	const reflection::Field &field)
{
	if (!VerifyFieldOfTable<T>(fbTable, field))
		return Nil();
	T i = flatbuffers::GetFieldI<T>(fbTable, field);
	return LuaRef::fromValue(L, i);
}

template<typename T>
LuaRef Decoder::DecodeFieldF(const Table& fbTable,
	const reflection::Field &field)
{
	if (!VerifyFieldOfTable<T>(fbTable, field))
		return Nil();
	T f = flatbuffers::GetFieldF<T>(fbTable, field);
	return LuaRef::fromValue(L, f);
}

LuaRef Decoder::DecodeTable(
	const reflection::Object& object,
	const Table& fbTable)
{
	assert(!object.is_struct());
	m_nameStack.Push(object.name()->str());
	if (!fbTable.VerifyTableStart(*m_pVerifier))
		ERR_RET_NIL("illegal start of table " + PopFullName());

	LuaRef luaTable = LuaRef::createTable(L);
	for (const reflection::Field* pField : *object.fields())
	{
		assert(pField);
		const char* pName = pField->name()->c_str();
		assert(pName);
		luaTable[pName] = DecodeFieldOfTable(fbTable, *pField);
		if (Bad()) return Nil();
	}

	if (!m_pVerifier->EndTable())
		ERR_RET_NIL("illegal end of table " + PopFullName());

	m_nameStack.SafePop();
	return luaTable;
}

LuaRef Decoder::DecodeStruct(const reflection::Object& object,
	const flatbuffers::Struct& fbStruct)
{
	assert(object.is_struct());
	m_nameStack.Push(object.name()->str());
	if (m_pVerifier->Verify(&fbStruct, object.bytesize()))
		ERR_RET_NIL("illegal struct " + PopFullName());

	LuaRef luaTable = LuaRef::createTable(L);
	for (const reflection::Field* pField : *object.fields())
	{
		assert(pField);
		const char* pName = pField->name()->c_str();
		assert(pName);
		luaTable[pName] = DecodeFieldOfStruct(fbStruct, *pField);
		if (Bad()) return Nil();
	}

	m_nameStack.SafePop();
	return luaTable;
}


LuaRef Decoder::DecodeVector(
	const reflection::Type& type,
	const flatbuffers::VectorOfAny& v)
{
	assert(reflection::Vector == type.base_type());
	reflection::BaseType elemType = type.element();
	assert(reflection::Vector != elemType && "Nesting vectors is not supported.");
	assert(reflection::Union != elemType && "Union must always be part of a table.");

	// Todo: Check key order.

	if (elemType <= reflection::Double)
		return DecodeScalarVector(elemType, v);
	if (reflection::String == elemType)
		return DecodeStringVector(v);
	if (reflection::Obj == elemType)
		return DecodeObjVector(*m_vObjects[type.index()], v);
	assert(!"Illegal element type.");
	return Nil();
}

LuaRef Decoder::DecodeScalarVector(reflection::BaseType elemType,
	const flatbuffers::VectorOfAny& v)
{
	assert(elemType <= reflection::Double);

	const uint8_t* end;
	if (!m_pVerifier->VerifyVector(
		reinterpret_cast<const uint8_t*>(&v),
		flatbuffers::GetTypeSize(elemType), &end))
		ERR_RET_NIL("illegal scalar vector " + PopFullName());

	LuaRef luaArray = LuaRef::createTable(L);
	const flatbuffers::VectorOfAny* pVec = &v;
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

LuaRef Decoder::DecodeStringVector(const flatbuffers::VectorOfAny& v)
{
	const uint8_t* end;
	if (!m_pVerifier->VerifyVector(reinterpret_cast<const uint8_t*>(&v),
		sizeof(flatbuffers::uoffset_t), &end))
		ERR_RET_NIL("illegal string vector " + PopFullName());

	LuaRef luaArray = LuaRef::createTable(L);
	for (size_t i = 0; i < v.size(); ++i)
	{
		const auto* pStr = flatbuffers::GetAnyVectorElemPointer<
			const flatbuffers::String>(&v, i);
		if (!m_pVerifier->Verify(pStr))
			ERR_RET_NIL("illegal string vector item " + PopFullVectorName(i));
		luaArray[i+1] = pStr->str();
	}
	return luaArray;
}

LuaRef Decoder::DecodeObjVector(const reflection::Object& elemObj,
	const flatbuffers::VectorOfAny& v)
{
	if (elemObj.is_struct())
		return DecodeStructVector(elemObj, v);
	return DecodeTableVector(elemObj, v);
}

LuaRef Decoder::DecodeStructVector(const reflection::Object& elemObj,
	const flatbuffers::VectorOfAny& v)
{
	assert(elemObj.is_struct());
	const uint8_t* end;
	if (!m_pVerifier->VerifyVector(reinterpret_cast<const uint8_t*>(&v),
		elemObj.bytesize(), &end))
		ERR_RET_NIL("illegal struct vector " + PopFullName());

	LuaRef luaArray = LuaRef::createTable(L);
	for (size_t i = 0; i < v.size(); ++i)
	{
		const auto* pStruct = flatbuffers::GetAnyVectorElemAddressOf<
			const flatbuffers::Struct>(&v, i, elemObj.bytesize());
		assert(pStruct);
		luaArray[i+1] = DecodeStruct(elemObj, *pStruct);
		if (Bad()) return Nil();
	}
	return luaArray;
}

LuaRef Decoder::DecodeTableVector(const reflection::Object& elemObj,
	const flatbuffers::VectorOfAny& v)
{
	assert(!elemObj.is_struct());
	const uint8_t* end;
	if (!m_pVerifier->VerifyVector(reinterpret_cast<const uint8_t*>(&v),
		sizeof(flatbuffers::uoffset_t), &end))
		ERR_RET_NIL("illegal table vector " + PopFullName());

	LuaRef luaArray = LuaRef::createTable(L);
	for (size_t i = 0; i < v.size(); ++i)
	{
		const Table* pTable = flatbuffers::GetAnyVectorElemPointer<
			const Table>(&v, i);
		if (!pTable)
			ERR_RET_NIL("illegal vector element point " + PopFullVectorName(i));
		luaArray[i+1] = DecodeTable(elemObj, *pTable);
		if (Bad()) return Nil();
	}
	return luaArray;
}

template <typename T>
inline LuaRef Decoder::DecodeScalar(const void* pData)
{
	assert(pData);
	if (!m_pVerifier->Verify<T>(pData))
		ERR_RET_NIL("illegal scalar " + PopFullName());
	return LuaRef::fromValue(L, flatbuffers::ReadScalar<T>(pData));
}

LuaRef Decoder::DecodeUnion(
	const reflection::Type& type,
	const void* pData)
{
	assert(pData);
	switch (type.base_type())
	{
	case reflection::UType:
	case reflection::Bool:
	case reflection::UByte:
		return DecodeScalar<uint8_t>(pData);
	case reflection::Byte:
		return DecodeScalar<int8_t>(pData);
	case reflection::Short:
		return DecodeScalar<int16_t>(pData);
	case reflection::UShort:
		return DecodeScalar<uint16_t>(pData);
	case reflection::Int:
		return DecodeScalar<uint8_t>(pData);
	case reflection::UInt:
		return DecodeScalar<uint8_t>(pData);
	case reflection::Long:
		return DecodeScalar<int64_t>(pData);
	case reflection::ULong:
		return DecodeScalar<uint64_t>(pData);
	case reflection::Float:
		return DecodeScalar<float>(pData);
	case reflection::Double:
		return DecodeScalar<double>(pData);

	case reflection::String:
	{
		const auto* pStr = reinterpret_cast<const flatbuffers::String*>(pData);
		if (!m_pVerifier->Verify(pStr))
			ERR_RET_NIL("illegal string " + PopFullName());
		return LuaRef::fromValue(L, pStr->str());
	}
	case reflection::Vector:
	{
		const auto* pVec = reinterpret_cast<const flatbuffers::VectorOfAny*>(pData);
		return DecodeVector(type, *pVec);
	}
	case reflection::Obj:
		return DecodeObject(*m_vObjects[type.index()], pData);
	case reflection::Union:
		assert(!"Union must always be part of a table.");
	}  // switch

	assert(false);
	return Nil();
}

template <typename T>
bool Decoder::VerifyFieldOfTable(
	const Table& fbTable, const reflection::Field &field)
{
	static_assert(std::is_scalar<T>::value, "T must be a scalar type");

	if (field.required())
	{
		if (fbTable.VerifyFieldRequired<T>(*m_pVerifier, field.offset()))
			return true;

		SetError("illegal required field "
			+ PopFullFieldName(field.name()->c_str()));
		return false;
	}

	if (fbTable.VerifyField<T>(*m_pVerifier, field.offset()))
		return true;

	SetError("illegal offset of field "
		+ PopFullFieldName(field.name()->c_str()));
	return false;
}

